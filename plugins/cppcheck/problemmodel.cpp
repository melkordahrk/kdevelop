/* This file is part of KDevelop

   Copyright 2017 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "problemmodel.h"

#include "plugin.h"
#include "utils.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iproject.h>
#include <language/editor/documentrange.h>
#include <shell/problemmodelset.h>
#include <qtcompat_p.h>

#include <klocalizedstring.h>

namespace cppcheck
{

inline KDevelop::ProblemModelSet* problemModelSet()
{
    return KDevelop::ICore::self()->languageController()->problemModelSet();
}

namespace Strings {
QString problemModelId() { return QStringLiteral("Cppcheck"); }
}

ProblemModel::ProblemModel(Plugin* plugin)
    : KDevelop::ProblemModel(plugin)
    , m_plugin(plugin)
    , m_project(nullptr)
    , m_pathLocation(KDevelop::DocumentRange::invalid())
{
    setFeatures(CanDoFullUpdate | ScopeFilter | SeverityFilter | Grouping | CanByPassScopeFilter);
    reset();
    problemModelSet()->addModel(Strings::problemModelId(), i18n("Cppcheck"), this);
}

ProblemModel::~ProblemModel()
{
    problemModelSet()->removeModel(Strings::problemModelId());
}

KDevelop::IProject* ProblemModel::project() const
{
    return m_project;
}

void ProblemModel::fixProblemFinalLocation(KDevelop::IProblem::Ptr problem)
{
    // Fix problems with incorrect range, which produced by cppcheck's errors
    // without <location> element. In this case location automatically gets "/".
    // To avoid this we set current analysis path as problem location.

    if (problem->finalLocation().document.isEmpty()) {
        problem->setFinalLocation(m_pathLocation);
    }

    const auto& diagnostics = problem->diagnostics();
    for (auto& diagnostic : diagnostics) {
        fixProblemFinalLocation(diagnostic);
    }
}

bool ProblemModel::problemExists(KDevelop::IProblem::Ptr newProblem)
{
    for (auto problem : qAsConst(m_problems)) {
        if (newProblem->source() == problem->source() &&
            newProblem->severity() == problem->severity() &&
            newProblem->finalLocation() == problem->finalLocation() &&
            newProblem->description() == problem->description() &&
            newProblem->explanation() == problem->explanation())
            return true;
    }

    return false;
}

void ProblemModel::setMessage(const QString& message)
{
    setPlaceholderText(message, m_pathLocation, i18n("Cppcheck"));
}

void ProblemModel::addProblems(const QVector<KDevelop::IProblem::Ptr>& problems)
{
    static int maxLength = 0;

    if (m_problems.isEmpty()) {
        maxLength = 0;
    }

    for (auto problem : problems) {
        fixProblemFinalLocation(problem);

        if (problemExists(problem)) {
            continue;
        }

        m_problems.append(problem);
        addProblem(problem);

        // This performs adjusting of columns width in the ProblemsView
        if (maxLength < problem->description().length()) {
            maxLength = problem->description().length();
            setProblems(m_problems);
        }
    }
}

void ProblemModel::setProblems()
{
    setMessage(i18n("Analysis completed, no problems detected."));
    setProblems(m_problems);
}

void ProblemModel::reset()
{
    reset(nullptr, QString());
}

void ProblemModel::reset(KDevelop::IProject* project, const QString& path)
{
    m_project = project;

    m_path = path;
    m_pathLocation.document = KDevelop::IndexedString(m_path);

    clearProblems();
    m_problems.clear();

    QString tooltip;
    if (m_project) {
        setMessage(i18n("Analysis started..."));
        tooltip = i18nc("@info:tooltip %1 is the path of the file", "Re-run last Cppcheck analysis (%1)", prettyPathName(m_path));
    } else {
        tooltip = i18nc("@info:tooltip", "Re-run last Cppcheck analysis");
    }

    setFullUpdateTooltip(tooltip);
}

void ProblemModel::show()
{
    problemModelSet()->showModel(Strings::problemModelId());
}

void ProblemModel::forceFullUpdate()
{
    if (m_project && !m_plugin->isRunning()) {
        m_plugin->runCppcheck(m_project, m_path);
    }
}

}
