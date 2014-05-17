#include "unknowndeclarationproblem.h"

#include "clangtypes.h"
#include "../debug.h"
#include "../util/clangutils.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/codegen/documentchangeset.h>
#include <language/interfaces/idefinesandincludesmanager.h>

#include <project/projectmodel.h>
#include <util/path.h>

#include <QDir>
#include <QProcess>

#include <KDebug>
#include <KLocale>

#include <algorithm>

using namespace KDevelop;

namespace {
/* Under some conditions, such as when looking up suggestions
 * for the undeclared namespace 'std' we will get an awful lot
 * of suggestions. This parameter limits how many suggestions
 * will pop up, as rarely more than a few will be relevant anyways
 *
 * Forward declaration suggestions are included in this number
 */
const int maxSuggestions = 5;

/*
 * We don't want anything from the bits directory -
 * we'd rather prefer forwarding includes, such as <vector>
 */
bool isBlacklisted( const QString& path ) {
    return path.contains( "bits" ) && path.contains( "/include/c++/" );
}

QStringList scanIncludePaths( const QString& identifier, const QDir& dir )
{
    QStringList candidates;
    const auto path = dir.absolutePath();

    if( isBlacklisted( path ) ) {
        return {};
    }

    /* Make this search case-insensitive? */
    for( const auto ext : { "", ".h", ".hpp", ".H", ".hh", "hxx", "tlh", "h++" } ) {
        if ( !dir.exists( identifier + ext ) ) {
            continue;
        }

        debug() << "Found candidate file" << path + "/" + identifier + ext;
        candidates.append( path + "/" + identifier + ext );
    }

    for( const auto& subdir : dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
        candidates += scanIncludePaths( identifier, QDir{ path + "/" + subdir } );

    return candidates;
}

/*
 * Find files in dir that match the given identifier. Matches common C++ header file extensions only.
 */
QStringList scanIncludePaths( const QualifiedIdentifier& identifier, const KDevelop::Path::List& includes )
{
    const auto stripped_identifier = identifier.last().toString();
    QStringList candidates;
    for( const auto& include : includes ) {
        candidates += scanIncludePaths( stripped_identifier, QDir{ include.toLocalFile() } );
    }

    std::sort( candidates.begin(), candidates.end() );
    candidates.erase( std::unique( candidates.begin(), candidates.end() ), candidates.end() );
    return candidates;
}

/*
 * Determine how much path is shared between two includes.
 *  boost/tr1/unordered_map
 *  boost/tr1/unordered_set
 * have a shared path of 2 where
 *  boost/tr1/unordered_map
 *  boost/vector
 * have a shared path of 1
 */
int sharedPathLevel(const QString& a, const QString& b)
{
    int shared = -1;
    for(auto x = a.begin(), y = b.begin(); *x == *y && x != a.end() && y != b.end() ; ++x, ++y ) {
        if( *x == QDir::separator() ) {
            ++shared;
        }
    }

    return shared;
}

/**
* Try to find a proper include position from the DUChain:
*
* look at existing imports (i.e. #include's) and find a fitting
* file with the same/similar path to the new include file and use that
*
* TODO: Implement a fallback scheme
*/
KDevelop::SimpleRange includeDirectivePosition( const KDevelop::Path& source, const QString& includeFile )
{
    DUChainReadLocker lock;
    const TopDUContext* top = DUChainUtils::standardContextForUrl( source.toUrl() );
    if( !top ) {
        debug() << "unable to find standard context for" << source.toLocalFile() << "Creating null range";
        return KDevelop::SimpleRange::invalid();
    }

    int line = -1;

    // look at existing #include statements and re-use them
    int currentMatchQuality = -1;
    for( const auto& import : top->importedParentContexts() ) {

        const int matchQuality = sharedPathLevel( import.context(top)->url().str(), includeFile );
        if( matchQuality < currentMatchQuality ) {
            continue;
        }

        line = import.position.line + 1;
        currentMatchQuality = matchQuality;
    }

    if( line == -1 ) {
        /* Insert at the top of the document */
        return KDevelop::SimpleRange{ 0, 0, 0, 0 };
    }

    return KDevelop::SimpleRange{ line, 0, line, 0 };
}

KDevelop::SimpleRange forwardDeclarationPosition( const KDevelop::Path& source )
{
    DUChainReadLocker lock;
    const TopDUContext* top = DUChainUtils::standardContextForUrl( source.toUrl() );
    if( !top ) {
        debug() << "unable to find standard context for" << source.toLocalFile() << "Creating null range";
        return KDevelop::SimpleRange::invalid();
    }

    int line = std::numeric_limits< int >::max();
    for( const auto decl : top->localDeclarations() ) {
        line = std::min( line, decl->range().start.line );
    }

    if( line == std::numeric_limits< int >::max() ) {
        return KDevelop::SimpleRange::invalid();
    }

    // We want it one line above the first declaration
    line = std::max( line - 1, 0 );

    return KDevelop::SimpleRange{ line, 0, line, 0 };
}

QVector<KDevelop::QualifiedIdentifier> possibleDeclarations( const QualifiedIdentifier& identifier, const KDevelop::Path& file, const KDevelop::CursorInRevision& cursor )
{
    DUChainReadLocker lock;
    const TopDUContext* top = DUChainUtils::standardContextForUrl( file.toUrl() );

    if( !top ) {
        debug() << "unable to find standard context for" << file.toLocalFile() << "Not creating duchain candidates";
        return {};
    }

    const auto* context = top->findContextAt( cursor );
    if( !context ) {
        debug() << "No context found at" << cursor;
        return {};
    }

    QVector<KDevelop::QualifiedIdentifier> declarations{ identifier };
    auto scopes = context->scopeIdentifier();

    /*
     * Iteratively build all levels of the current scope. A (missing) type anywhere
     * can be aribtrarily namespaced, so we create the permutations of possible
     * nestings of namespaces it can currently be in,
     *
     * TODO: add detection of namespace aliases, such as 'using namespace KDevelop;'
     *
     * namespace foo {
     *      namespace bar {
     *          function baz() {
     *              type var;
     *          }
     *      }
     * }
     *
     * Would give:
     * foo::bar::baz::type
     * foo::bar::type
     * foo::type
     * type
     */
    for( auto scopes = context->scopeIdentifier(); !scopes.isEmpty(); scopes.pop() ) {
        declarations.append( scopes + identifier );
    }

    debug() << "Possible declarations:" << declarations;
    return declarations;
}

QStringList duchainCandidates( const QualifiedIdentifier& identifier, const KDevelop::Path& file, const KDevelop::CursorInRevision& cursor )
{
    DUChainReadLocker lock;
    /*
     * Search the persistent symbol table for the declaration. If it is known from before,
     * determine which file it came from and suggest that
     */
    QStringList candidates;
    for( const auto& declaration : possibleDeclarations( identifier, file, cursor ) ) {
        debug() << "Considering candidate declaration" << declaration;
        const IndexedDeclaration* declarations;
        uint declarationCount;
        PersistentSymbolTable::self().declarations( declaration , declarationCount, declarations );

        for( uint i = 0; i < declarationCount; ++i ) {
            auto* decl = declarations[ i ].declaration();

            /* Skip if the declaration is invalid or if it is an alias declaration -
             * we want the actual declaration (and its file)
             */
            if( !decl ) {
                continue;
            }
            if( dynamic_cast<KDevelop::AliasDeclaration*>( decl ) ) {
                continue;
            }

            if( decl->isForwardDeclaration() ) {
                continue;
            }

            const auto filepath = decl->url().toUrl().toLocalFile();

            if( !isBlacklisted( filepath ) ) {
                candidates << filepath;
                debug() << "Adding" << filepath << "determined from candidate" << declaration;
            }

            for( const auto importer : decl->topContext()->parsingEnvironmentFile()->importers() ) {
                if( importer->imports().count() != 1 && !isBlacklisted( filepath ) ) {
                    continue;
                }
                if( importer->topContext()->localDeclarations().count() ) {
                    continue;
                }

                const auto filePath = importer->url().toUrl().toLocalFile();
                if( isBlacklisted( filePath ) ) {
                    continue;
                }

                /* This file is a forwarder, such as <vector>
                 * <vector> does not actually implement the functions, but include other headers that do
                 * we prefer this to other headers
                 */
                candidates << filePath;
                debug() << "Adding forwarder file" << filePath << "to the result set";
            }
        }
    }

    std::sort( candidates.begin(), candidates.end() );
    candidates.erase( std::unique( candidates.begin(), candidates.end() ), candidates.end() );
    debug() << "Candidates: " << candidates;
    return candidates;
}

/*
 * Takes a filepath and the include paths and determines what directive to use.
 */
QString directiveForFile( const QString& includefile, const KDevelop::Path::List& includepaths, const KDevelop::Path& source )
{
    QString shortestDirective;
    bool isRelative = false;
    const auto current = source.parent();

    const Path canonicalFile( QFileInfo( includefile ).canonicalFilePath() );

    for( const auto& includePath : includepaths ) {
        QString relative = includePath.relativePath( canonicalFile );
        if( relative.startsWith( "./" ) )
            relative = relative.mid( 2 );

        if( shortestDirective.isEmpty() || relative.length() < shortestDirective.length() ) {
            shortestDirective = relative;
            isRelative = includePath == current;
        }
    }

    if( shortestDirective.isEmpty() ) {
        // Item not found in include path
        return {};
    }

    if( isRelative ) {
        return QString{ "#include \"%1\"" }.arg( shortestDirective );
    }

    return QString{ "#include <%1>" }.arg( shortestDirective );
}

KDevelop::Path::List includePaths( const KDevelop::Path& file )
{
    /*
     * Find project's custom include paths
     */
    const auto source = file.toLocalFile();
    const auto item = ICore::self()->projectController()->projectModel()->itemForPath( KDevelop::IndexedString( source ) );

    return IDefinesAndIncludesManager::manager()->includes(item);
}

/*
 * Return a list of header files viable for inclusions. All elements will be unique
 */
QStringList includeFiles( const QualifiedIdentifier& identifier, const KDevelop::Path& file, const KDevelop::DocumentRange& range )
{
    const CursorInRevision cursor{ range.start.line, range.start.column };
    const auto includes = includePaths( file );

    if( includes.isEmpty() ) {
        debug() << "Include path is empty";
        return {};
    }

    const auto candidates = duchainCandidates( identifier, file, cursor );
    if( !candidates.isEmpty() ) {
        // If we find a candidate from the duchain we don't bother scanning the include paths
        return candidates;
    }

    return scanIncludePaths( identifier, includes );
}

QStringList forwardDeclarations( const QualifiedIdentifier& identifier )
{
    /*
     *  Construct viable forward declarations for the type name.
     *
     * Currently we're not able to determine what is namespaces, class names etc
     * and makes a suitable forward declaration, so just suggest "vanilla" declarations.
     */
    const auto name = identifier.last().toString();
    //return QStringList{} << "class " + name + ";" << "struct " + name + ";";
    return { "class " + name + ";", "struct " + name + ";" };
}

ClangFixits fixUnknownDeclaration( const QualifiedIdentifier& identifier, const KDevelop::Path& file, const KDevelop::DocumentRange& docrange )
{
    ClangFixits fixits;

    const auto forwardDeclRange = forwardDeclarationPosition( file );
    for( const auto& decl : forwardDeclarations( identifier ) ) {
        /* Determine some location for forward decls */
        if( !forwardDeclRange.isValid() ) {
            continue;
        }
        fixits << ClangFixit{ decl, DocumentRange(IndexedString(file.pathOrUrl()), forwardDeclRange), QString() };
    }

    const auto includepaths = includePaths( file );
    const auto includefiles = includeFiles( identifier, file, docrange );

    /* create fixits for candidates */
    for( const auto& includeFile : includefiles ) {
        const auto directive = directiveForFile( includeFile, includepaths, file /* UP */ );
        if( directive.isEmpty() ) {
            debug() << "unable to create directive for" << includeFile << "in" << file.toLocalFile();
            continue;
        }

        const auto range = includeDirectivePosition( file, includeFile );
        if( !range.isValid() ) {
            debug() << "unable to determine valid position for" << directive << "in" << file.toLocalFile();
            continue;
        }

        fixits << ClangFixit{ directive, DocumentRange(IndexedString(file.pathOrUrl()), range), QString() };
    }

    if( fixits.size() > maxSuggestions ) {
        fixits.resize( maxSuggestions );
    }

    return fixits;
}

QString symbolFromDiagnosticSpelling(const QString& str)
{
    /* in all error messages the symbol is in in the first pair of quotes */
    const auto split = str.split( '\'' );
    auto symbol = split.value( 1 );

    if( str.startsWith( "No member named" ) ) {
        symbol = split.value( 3 ) + "::" + split.value( 1 );
    }
    return symbol;
}

}

UnknownDeclarationProblem::UnknownDeclarationProblem(CXDiagnostic diagnostic)
    : ClangProblem(diagnostic)
{
    setSymbol(QualifiedIdentifier(symbolFromDiagnosticSpelling(description())));
}

void UnknownDeclarationProblem::setSymbol(const QualifiedIdentifier& identifier)
{
    m_identifier = identifier;
}

KSharedPtr<IAssistant> UnknownDeclarationProblem::solutionAssistant() const
{
    const Path path(finalLocation().document.str());
    const auto fixits = allFixits() + fixUnknownDeclaration(m_identifier, path, finalLocation());
    return KSharedPtr<IAssistant>(new ClangFixitAssistant(fixits));
}
