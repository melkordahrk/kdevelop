/***************************************************************************
                     main.cpp - the main-function
                             -------------------

    begin                : 20 Jul 1998
    copyright            : (C) 1998 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ckdevelop.h"
#include "ckdevinstall.h"
#include "kstartuplogo.h"

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapp.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <kmessagebox.h>

static const char *description =
        I18N_NOOP("The KDevelop C/C++ Integrated Development Environment");

static KCmdLineOptions options[] =
{
   { "setup", I18N_NOOP("Setup KDevelop"), 0 },
   { "+[file]", I18N_NOOP("The file to open"), 0 },
   { 0, 0, 0 }
};

int main(int argc, char* argv[])
{
  KConfig* config;
  KStartupLogo* start_logo;

  KAboutData aboutData( "kdevelop",
                        I18N_NOOP("KDevelop"),
                        VERSION,
                        description,
                        KAboutData::License_GPL,
                        "(c) 1998-2001, The KDevelop Team",
                        I18N_NOOP("The KDevelop C/C++ IDE - a success story "
                                  "in the world of free software since 1998.\n\n"
                                  "Think about joining our project to contribute "
                                  "and enhance the product you are using for your\n"
                                  "everyday work - or think about writing a KDE "
                                  "application that will contribute to the KDE project.\n\n"
                                  "For questions about this product, its usage and license issues,\n"
                                  "please contact the KDE project (http://www.kde.org) or any \n"
                                  "of the team officials in the Authors section."),
                                  "http://www.kdevelop.org"  );

  aboutData.addAuthor("Ralf Nolden",I18N_NOOP("Maintainer, Configuration Functionality,"
                          "Online Help, Popups, port of KDevelop 1.3 to KDE 2.1"),"nolden@kde.org");
  aboutData.addAuthor("Sandy Meier",I18N_NOOP("former Maintainer"), "smeier@kdevelop.org");
  aboutData.addAuthor("Walter Tasin",I18N_NOOP("Many, Many Bugfixes, General Enhancements"), "tasin@kdevelop.org");
  aboutData.addAuthor("Bernd Gehrmann",I18N_NOOP("Documentation Tree"), "bernd@physik.hu-berlin.de");
  aboutData.addAuthor("Jonas Nordin",I18N_NOOP("Classviewer and Classparser"), "jonas.nordin@syncom.se");
  aboutData.addAuthor("J�rgen Olsson",I18N_NOOP("Graphical Classviewer"),"jorgen@trej.net");
  aboutData.addAuthor("Stefan Heidrich",I18N_NOOP("KAppWizard, Printing"), "sheidric@rz.uni-potsdam.de");
  aboutData.addAuthor("Stefan Bartel",I18N_NOOP("Real-File-Viewer, Project Options"), "bartel@rz.uni-potsdam.de");
  aboutData.addAuthor("John Birch",I18N_NOOP("Internal Debugger, port of KDevelop 1.3 to KDE 2.1"), "jbb@kdevelop.org");
	aboutData.addAuthor("F@lk Brettschneider",I18N_NOOP("MDI"), "falk.brettschneider@gmx.de", "http://www.geocities.com/gigafalk/qextmdi.htm");

  aboutData.addCredit ("Jochen Wilhelmy",I18N_NOOP("Initial KWrite sources"), "digisnap@cs.tu-berlin.de");
  aboutData.addCredit ("Martin R. Jones",I18N_NOOP("KDE Help"), "mjones@kde.org");
  aboutData.addCredit ("Matthias Hoelzer",I18N_NOOP("KSwallow, kcmlocale"), "hoelzer@physik.uni-wuerzburg.de");
  aboutData.addCredit ("Havoc Pennington",I18N_NOOP("Gnome Template Application"), "rhp@zirx.pair.com");
	aboutData.addCredit ("Ian Reinhart Geiser",I18N_NOOP("Kicker templates and the RPM build facility"), "geiseri@linuxppc.com");

  aboutData.addCredit ("Pau Estalella Fernandez",I18N_NOOP("Patches/Bugfixes"), "pef@upcnet.upc.es");
  aboutData.addCredit ("Jost Schenk",I18N_NOOP("Patches/Bugfixes"), "Jost@Schenk.de");
  aboutData.addCredit ("David Barth",I18N_NOOP("Patches/Bugfixes"), "dbarth@videotron.ca");
  aboutData.addCredit ("Matthias Hipp",I18N_NOOP("Patches/Bugfixes"), "Matthias.Hipp@gmx.de");
  aboutData.addCredit ("Matthias Hoelzer-Kluepfel",I18N_NOOP("Patches/Bugfixes"), "mhk@caldera.de");
  aboutData.addCredit ("Matt Koss",I18N_NOOP("Patches/Bugfixes"), "koss@napri.sk");
  aboutData.addCredit ("Torsten Uhlmann",I18N_NOOP("Patches/Bugfixes"), "TUhlmann@debis.com");
  aboutData.addCredit ("Stefan Mars",I18N_NOOP("Patches/Bugfixes"), "mars@lysator.liu.se");
  aboutData.addCredit ("Nikolay Liber",I18N_NOOP("Patches/Bugfixes"), "nikolay_liber@mail.ru");
  aboutData.addCredit ("Gordon Tyler",I18N_NOOP("Patches/Bugfixes"), "gtyler@iafrica.com");
  aboutData.addCredit ("Huy Cuong Nguyen",I18N_NOOP("Patches/Bugfixes"), "huyc@iquebec.com");
  aboutData.addCredit ("John R. Zedlewski",I18N_NOOP("Patches/Bugfixes"), "zedlwski@princeton.edu");
  aboutData.addCredit ("Stephan Uhlmann",I18N_NOOP("Patches/Bugfixes"), "suhlmann@gmx.de");
  aboutData.addCredit ("Charles Egan",I18N_NOOP("Patches/Bugfixes"), "cega@oklahoma.net");
  aboutData.addCredit ("Andrew Morton",I18N_NOOP("Patches/Bugfixes"), "morton@nortelnetworks.com");
  aboutData.addCredit ("p_george",I18N_NOOP("Patches/Bugfixes"), "p_george@club-internet.fr");
  aboutData.addCredit ("Lubos Lunak",I18N_NOOP("Patches/Bugfixes"), "l.lunak@sh.cvut.cz");
  aboutData.addCredit ("Martin Spirk",I18N_NOOP("Patches/Bugfixes"), "spirk@kla.pvt.cz");
  aboutData.addCredit ("Ralf Palsa",I18N_NOOP("Patches/Bugfixes"), "rpalsa@eso.org");
  aboutData.addCredit ("Richard Dale",I18N_NOOP("Patches/Bugfixes"), "Richard_Dale@tipitina.demon.co.uk");
  aboutData.addCredit ("Holger Schurig",I18N_NOOP("Patches/Bugfixes"), "holger@holger.om.org");
  aboutData.addCredit ("Dag Andersen",I18N_NOOP("Patches/Bugfixes"), "dandersen@c2i.net");
  aboutData.addCredit ("Alexis Mikhailov",I18N_NOOP("Patches/Bugfixes"), "alexis@abc.cap.ru");
  aboutData.addCredit ("Ulltveit-Moe",I18N_NOOP("Patches/Bugfixes"), "Nils.Ulltveit-Moe@hia.no");
  aboutData.addCredit ("Nich Hudson",I18N_NOOP("Patches/Bugfixes"), "nick@nthcliff.demon.co.uk");
  aboutData.addCredit ("R. Imura",I18N_NOOP("Patches/Bugfixes"), "imura@cs.titech.ac.jp");
  aboutData.addCredit ("Ralf Funken",I18N_NOOP("Patches/Bugfixes"), "rfunken@kdevelop.de");
  aboutData.addCredit ("Jens Zurheide",I18N_NOOP("Patches/Bugfixes"), "zuzu@surf-callino.de");
  aboutData.addCredit ("Micha Bieber",I18N_NOOP("Patches/Bugfixes"), "micha@ani.de");
  aboutData.addCredit ("August Hoerandl",I18N_NOOP("Patches/Bugfixes"), "august.hoerandl@gmx.at");
  aboutData.addCredit ("Serge Lussier",I18N_NOOP("Patches/Bugfixes"), "serge.lussier@videotron.ca");
  aboutData.addCredit ("Roland Krause",I18N_NOOP("Patches/Bugfixes"), "rokrau@yahoo.com");
  aboutData.addCredit ("Daniel Engelschalt",I18N_NOOP("Patches/Bugfixes"), "s2697@htw-dresden.de");
  aboutData.addCredit ("Harald Fernengel",I18N_NOOP("Patches/Bugfixes"), "harry@bnro.de");
  aboutData.addCredit ("Christian Chouder",I18N_NOOP("Patches/Bugfixes"), "chcouder@club-internet.fr");
  aboutData.addCredit ("Ravikiran Rajagopal",I18N_NOOP("Patches/Bugfixes"), "ravi@ee.eng.ohio-state.edu");

  aboutData.addCredit ("Jacek Wojdel",I18N_NOOP("Startlogo KDevelop 1.0"), "wojdel@kbs.twi.tudelft.nl");
  aboutData.addCredit ("Ralph Kocher",I18N_NOOP("Startlogo KDevelop 1.1, 1.2"), "RKocher@t-online.de");
	
  aboutData.addCredit ("Martin Piskernig",I18N_NOOP("Patches/Bugfixes, Translation coordination"), "martin.piskernig@stuwo.at");
	
	KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication a;
  a.dcopClient()->attach();
  a.dcopClient()->registerAs("kdevelop");

#ifndef USE_KDE_2_1_1
  // KDE version check
  KStandardDirs stddirs;
  QString libPath = stddirs.findResourceDir("lib", "libkdeui.so.3.0.0");
  QFileInfo libFI( libPath + "/libkdeui.so.3.0.0");
  QDateTime requiredDate(QDate(2001,5,24), QTime(22,30));
  if ( libFI.lastModified() < requiredDate) {
    KMessageBox::sorry(0L,
                       i18n("KDevelop cannot start :-(\n") +
                       i18n("because your KDE base library libkdeui.so is too old!\n\n") +
                       i18n("You need a version newer than %1\n but the detected version is from %2.\n").arg(requiredDate.toString()).arg(libFI.lastModified().toString()),
                       i18n("KDE version") );
    ::exit(0);
  }
#endif

  config = KGlobal::config();
  config->setGroup("General Options");
  bool bInstall=config->readBoolEntry("Install",true);

  if (args->isSet("setup"))
    bInstall = true; // start the setupwizard

//  if (kapp->isRestored())
//  {
//    RESTORE(CKDevelop);
//  }
//  else
  {
    if (bInstall)
    {
      CKDevInstall* install=new CKDevInstall(0,"install");
      install->show();
      delete install;
    }

  config->setGroup("General Options");
  start_logo=NULL;
  if (config->readBoolEntry("Logo",true) && (!kapp->isRestored() ) )
  {
    start_logo= new KStartupLogo();
    start_logo->show();
    start_logo->raise();
    QApplication::flushX();
  }

    CKDevelop* kdevelop = new CKDevelop();
    a.setMainWidget(kdevelop);

   if (start_logo)
	delete start_logo;

    kdevelop->completeStartup(args->count() == 0);

    if (bInstall)
      kdevelop->refreshTrees();  // this is because of the new documentation

    config->setGroup("General Options");
    kdevelop->slotTCurrentTab(config->readNumEntry("LastActiveTree",DOC));
    
    if (args->count())
      kdevelop->slotProjectOpenCmdl(args->arg(0));

    kdevelop->bStartupIsPending = false;  // see queryClose()
  }
  
  args->clear();

  int rc = a.exec();
  return rc;
}
