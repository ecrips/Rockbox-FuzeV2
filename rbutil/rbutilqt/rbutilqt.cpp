/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 *   Copyright (C) 2007 by Dominik Riebeling
 *   $Id$
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include <QtGui>

#include "version.h"
#include "rbutilqt.h"
#include "ui_rbutilqtfrm.h"
#include "ui_aboutbox.h"
#include "configure.h"
#include "install.h"
#include "installtalkwindow.h"
#include "createvoicewindow.h"
#include "httpget.h"
#include "themesinstallwindow.h"
#include "uninstallwindow.h"
#include "utils.h"
#include "rbzip.h"
#include "sysinfo.h"
#include "detect.h"
#include "rbsettings.h"

#include "progressloggerinterface.h"

#include "bootloaderinstallbase.h"
#include "bootloaderinstallmi4.h"
#include "bootloaderinstallhex.h"
#include "bootloaderinstallipod.h"
#include "bootloaderinstallsansa.h"
#include "bootloaderinstallfile.h"

#if defined(Q_OS_LINUX)
#include <stdio.h>
#endif
#if defined(Q_OS_WIN32)
#if defined(UNICODE)
#define _UNICODE
#endif
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#endif

RbUtilQt::RbUtilQt(QWidget *parent) : QMainWindow(parent)
{
    absolutePath = qApp->applicationDirPath();

    HttpGet::setGlobalUserAgent("rbutil/"VERSION);
    // init startup & autodetection
    ui.setupUi(this);
    updateSettings();
    downloadInfo();

    m_gotInfo = false;
    m_auto = false;

    // manual tab
    ui.radioPdf->setChecked(true);

    // info tab
    ui.treeInfo->setAlternatingRowColors(true);
    ui.treeInfo->setHeaderLabels(QStringList() << tr("File") << tr("Version"));
    ui.treeInfo->expandAll();
    ui.treeInfo->setColumnCount(2);
    // disable quick install until version info is available
    ui.buttonSmall->setEnabled(false);
    ui.buttonComplete->setEnabled(false);

    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateTabs(int)));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui.action_About, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.action_Help, SIGNAL(triggered()), this, SLOT(help()));
    connect(ui.action_Configure, SIGNAL(triggered()), this, SLOT(configDialog()));
    connect(ui.buttonChangeDevice, SIGNAL(clicked()), this, SLOT(configDialog()));
    connect(ui.buttonRockbox, SIGNAL(clicked()), this, SLOT(installBtn()));
    connect(ui.buttonBootloader, SIGNAL(clicked()), this, SLOT(installBootloaderBtn()));
    connect(ui.buttonFonts, SIGNAL(clicked()), this, SLOT(installFontsBtn()));
    connect(ui.buttonGames, SIGNAL(clicked()), this, SLOT(installDoomBtn()));
    connect(ui.buttonTalk, SIGNAL(clicked()), this, SLOT(createTalkFiles()));
    connect(ui.buttonCreateVoice, SIGNAL(clicked()), this, SLOT(createVoiceFile()));
    connect(ui.buttonVoice, SIGNAL(clicked()), this, SLOT(installVoice()));
    connect(ui.buttonThemes, SIGNAL(clicked()), this, SLOT(installThemes()));
    connect(ui.buttonRemoveRockbox, SIGNAL(clicked()), this, SLOT(uninstall()));
    connect(ui.buttonRemoveBootloader, SIGNAL(clicked()), this, SLOT(uninstallBootloader()));
    connect(ui.buttonDownloadManual, SIGNAL(clicked()), this, SLOT(downloadManual()));
    connect(ui.buttonSmall, SIGNAL(clicked()), this, SLOT(smallInstall()));
    connect(ui.buttonComplete, SIGNAL(clicked()), this, SLOT(completeInstall()));

    // actions accessible from the menu
    connect(ui.actionComplete_Installation, SIGNAL(triggered()), this, SLOT(completeInstall()));
    connect(ui.actionSmall_Installation, SIGNAL(triggered()), this, SLOT(smallInstall()));
    connect(ui.actionInstall_Bootloader, SIGNAL(triggered()), this, SLOT(installBootloaderBtn()));
    connect(ui.actionInstall_Rockbox, SIGNAL(triggered()), this, SLOT(installBtn()));
    connect(ui.actionFonts_Package, SIGNAL(triggered()), this, SLOT(installFontsBtn()));
    connect(ui.actionInstall_Themes, SIGNAL(triggered()), this, SLOT(installThemes()));
    connect(ui.actionInstall_Game_Files, SIGNAL(triggered()), this, SLOT(installDoomBtn()));
    connect(ui.actionInstall_Voice_File, SIGNAL(triggered()), this, SLOT(installVoice()));
    connect(ui.actionCreate_Voice_File, SIGNAL(triggered()), this, SLOT(createVoiceFile()));
    connect(ui.actionCreate_Talk_Files, SIGNAL(triggered()), this, SLOT(createTalkFiles()));
    connect(ui.actionRemove_bootloader, SIGNAL(triggered()), this, SLOT(uninstallBootloader()));
    connect(ui.actionUninstall_Rockbox, SIGNAL(triggered()), this, SLOT(uninstall()));
    connect(ui.action_System_Info, SIGNAL(triggered()), this, SLOT(sysinfo()));

#if !defined(STATIC)
    ui.actionInstall_Rockbox_Utility_on_player->setEnabled(false);
#else
    connect(ui.actionInstall_Rockbox_Utility_on_player, SIGNAL(triggered()), this, SLOT(installPortable()));
#endif

}


void RbUtilQt::sysinfo(void)
{
    Sysinfo *info = new Sysinfo(this);
    info->show();
}

void RbUtilQt::updateTabs(int count)
{
    switch(count) {
        case 6:
            updateInfo();
            break;
        default:
            break;
    }
}


void RbUtilQt::downloadInfo()
{
    // make sure the version map is repopulated correctly later.
    versmap.clear();
    // try to get the current build information
    daily = new HttpGet(this);
    connect(daily, SIGNAL(done(bool)), this, SLOT(downloadDone(bool)));
    connect(daily, SIGNAL(requestFinished(int, bool)), this, SLOT(downloadDone(int, bool)));
    connect(qApp, SIGNAL(lastWindowClosed()), daily, SLOT(abort()));
    if(RbSettings::value(RbSettings::CacheOffline).toBool())
        daily->setCache(true);
    else
        daily->setCache(false);
    qDebug() << "downloading build info";
    daily->setFile(&buildInfo);
    daily->getFile(QUrl(RbSettings::value(RbSettings::ServerConfUrl).toString()));
}


void RbUtilQt::downloadDone(bool error)
{
    if(error) {
        qDebug() << "network error:" << daily->error();
        QMessageBox::critical(this, tr("Network error"),
            tr("Can't get version information."));
        return;
    }
    qDebug() << "network status:" << daily->error();

    buildInfo.open();
    QSettings info(buildInfo.fileName(), QSettings::IniFormat, this);
    buildInfo.close();
    versmap.insert("arch_rev", info.value("dailies/rev").toString());
    versmap.insert("arch_date", info.value("dailies/date").toString());

    info.beginGroup("release");
    versmap.insert("rel_rev", info.value(RbSettings::value(RbSettings::CurBuildserverModel).toString()).toString());
    info.endGroup();

    if(versmap.value("rel_rev").isEmpty()) {
        ui.buttonSmall->setEnabled(false);
        ui.buttonComplete->setEnabled(false);
    }
    else {
        ui.buttonSmall->setEnabled(true);
        ui.buttonComplete->setEnabled(true);
    }

    bleeding = new HttpGet(this);
    connect(bleeding, SIGNAL(done(bool)), this, SLOT(downloadBleedingDone(bool)));
    connect(bleeding, SIGNAL(requestFinished(int, bool)), this, SLOT(downloadDone(int, bool)));
    connect(qApp, SIGNAL(lastWindowClosed()), daily, SLOT(abort()));
    if(RbSettings::value(RbSettings::CacheOffline).toBool())
        bleeding->setCache(true);
    bleeding->setFile(&bleedingInfo);
    bleeding->getFile(QUrl(RbSettings::value(RbSettings::BleedingInfo).toString()));

    if(RbSettings::value(RbSettings::RbutilVersion) != PUREVERSION) {
        QApplication::processEvents();
        QMessageBox::information(this, tr("New installation"),
            tr("This is a new installation of Rockbox Utility, or a new version. "
                "The configuration dialog will now open to allow you to setup the program, "
                " or review your settings."));
        configDialog();
    }
    else if(chkConfig(false)) {
        QApplication::processEvents();
        QMessageBox::critical(this, tr("Configuration error"),
            tr("Your configuration is invalid. This is most likely due "
                "to a changed device path. The configuration dialog will "
                "now open to allow you to correct the problem."));
        configDialog();
    }
}


void RbUtilQt::downloadBleedingDone(bool error)
{
    if(error) {
        qDebug() << "network error:" << bleeding->error();
    }
    else {
        bleedingInfo.open();
        QSettings info(bleedingInfo.fileName(), QSettings::IniFormat, this);
        bleedingInfo.close();
        versmap.insert("bleed_rev", info.value("bleeding/rev").toString());
        versmap.insert("bleed_date", info.value("bleeding/timestamp").toString());
        qDebug() << "versmap =" << versmap;

        m_gotInfo = true;
    }
}


void RbUtilQt::downloadDone(int id, bool error)
{
    QString errorString;
    errorString = tr("Network error: %1. Please check your network and proxy settings.")
        .arg(daily->errorString());
    if(error) {
        QMessageBox::about(this, "Network Error", errorString);
        m_networkerror = daily->errorString();
    }
    qDebug() << "downloadDone:" << id << "error:" << error;
}


void RbUtilQt::about()
{
    QDialog *window = new QDialog(this);
    Ui::aboutBox about;
    about.setupUi(window);
    window->setModal(true);

    QFile licence(":/docs/gpl-2.0.html");
    licence.open(QIODevice::ReadOnly);
    QTextStream c(&licence);
    QString cline = c.readAll();
    about.browserLicense->insertHtml(cline);
    about.browserLicense->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    QFile credits(":/docs/CREDITS");
    credits.open(QIODevice::ReadOnly);
    QTextStream r(&credits);
    r.setCodec(QTextCodec::codecForName("UTF-8"));
    QString rline = r.readAll();
    about.browserCredits->insertPlainText(rline);
    about.browserCredits->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    QString title = QString("<b>The Rockbox Utility</b><br/>Version %1").arg(FULLVERSION);
    about.labelTitle->setText(title);
    about.labelHomepage->setText("<a href='http://www.rockbox.org'>http://www.rockbox.org</a>");

    window->show();

}


void RbUtilQt::help()
{
    QUrl helpurl("http://www.rockbox.org/wiki/RockboxUtility");
    QDesktopServices::openUrl(helpurl);
}


void RbUtilQt::configDialog()
{
    Config *cw = new Config(this);
    connect(cw, SIGNAL(settingsUpdated()), this, SLOT(updateSettings()));
    connect(cw, SIGNAL(settingsUpdated()), this, SLOT(downloadInfo()));
    cw->show();
}


void RbUtilQt::updateSettings()
{
    qDebug() << "updateSettings()";
    updateDevice();
    updateManual();
    if(RbSettings::value(RbSettings::ProxyType) == "system") {
        HttpGet::setGlobalProxy(Detect::systemProxy());
    }
    else if(RbSettings::value(RbSettings::ProxyType) == "manual") {
        HttpGet::setGlobalProxy(RbSettings::value(RbSettings::Proxy).toString());
    }
    else {
        HttpGet::setGlobalProxy(QUrl(""));
    }
    HttpGet::setGlobalCache(RbSettings::value(RbSettings::CachePath).toString());
    HttpGet::setGlobalDumbCache(RbSettings::value(RbSettings::CacheOffline).toBool());
}


void RbUtilQt::updateDevice()
{
    if(RbSettings::value(RbSettings::CurBootloaderMethod) == "none" ) {
        ui.buttonBootloader->setEnabled(false);
        ui.buttonRemoveBootloader->setEnabled(false);
        ui.labelBootloader->setEnabled(false);
        ui.labelRemoveBootloader->setEnabled(false);
    }
    else {
        ui.buttonBootloader->setEnabled(true);
        ui.labelBootloader->setEnabled(true);
        if(RbSettings::value(RbSettings::CurBootloaderMethod) == "fwpatcher") {
            ui.labelRemoveBootloader->setEnabled(false);
            ui.buttonRemoveBootloader->setEnabled(false);
        }
        else {
            ui.labelRemoveBootloader->setEnabled(true);
            ui.buttonRemoveBootloader->setEnabled(true);
        }
    }

    // displayed device info
    QString mountpoint = RbSettings::value(RbSettings::Mountpoint).toString();
    QString brand = RbSettings::value(RbSettings::CurBrand).toString();
    QString name = RbSettings::value(RbSettings::CurName).toString();
    if(name.isEmpty()) name = "&lt;none&gt;";
    if(mountpoint.isEmpty()) mountpoint = "&lt;invalid&gt;";
    ui.labelDevice->setText(tr("<b>%1 %2</b> at <b>%3</b>")
            .arg(brand, name, QDir::toNativeSeparators(mountpoint)));
}


void RbUtilQt::updateManual()
{
    if(RbSettings::value(RbSettings::Platform) != "")
    {
        QString manual= RbSettings::value(RbSettings::CurManual).toString();

        if(manual == "")
            manual = "rockbox-" + RbSettings::value(RbSettings::Platform).toString();
        QString pdfmanual;
        pdfmanual = RbSettings::value(RbSettings::ManualUrl).toString()
                            + "/" + manual + ".pdf";
        QString htmlmanual;
        htmlmanual = RbSettings::value(RbSettings::ManualUrl).toString()
                            + "/" + manual + "/rockbox-build.html";
        ui.labelPdfManual->setText(tr("<a href='%1'>PDF Manual</a>")
            .arg(pdfmanual));
        ui.labelHtmlManual->setText(tr("<a href='%1'>HTML Manual (opens in browser)</a>")
            .arg(htmlmanual));
    }
    else {
        ui.labelPdfManual->setText(tr("Select a device for a link to the correct manual"));
        ui.labelHtmlManual->setText(tr("<a href='%1'>Manual Overview</a>")
            .arg("http://www.rockbox.org/manual.shtml"));

    }
}

void RbUtilQt::completeInstall()
{
    if(chkConfig(true)) return;
    if(QMessageBox::question(this, tr("Confirm Installation"),
           tr("Do you really want to perform a complete installation?\n\n"
              "This will install Rockbox %1. To install the most recent "
              "development build available press \"Cancel\" and "
              "use the \"Installation\" tab.")
              .arg(versmap.value("rel_rev")),
              QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
        return;
    // create logger
    logger = new ProgressLoggerGui(this);
    logger->show();

    if(smallInstallInner())
        return;
    logger->setRunning();
    // Fonts
    m_error = false;
    m_installed = false;
    if(!installFontsAuto())
        return;
    else
    {
        // wait for installation finished
        while(!m_installed)
           QApplication::processEvents();
    }
    if(m_error) return;
    logger->setRunning();

    // Doom
    if(hasDoom())
    {
        m_error = false;
        m_installed = false;
        if(!installDoomAuto())
            return;
        else
        {
            // wait for installation finished
            while(!m_installed)
               QApplication::processEvents();
        }
        if(m_error) return;
    }

    // theme
    // this is a window
    // it has its own logger window,so close our.
    logger->close();
    installThemes();

}

void RbUtilQt::smallInstall()
{
    if(chkConfig(true)) return;
    if(QMessageBox::question(this, tr("Confirm Installation"),
           tr("Do you really want to perform a minimal installation? "
              "A minimal installation will contain only the absolutely "
              "necessary parts to run Rockbox.\n\n"
              "This will install Rockbox %1. To install the most recent "
              "development build available press \"Cancel\" and "
              "use the \"Installation\" tab.")
              .arg(versmap.value("rel_rev")),
              QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    // create logger
    logger = new ProgressLoggerGui(this);
    logger->show();

    smallInstallInner();
}

bool RbUtilQt::smallInstallInner()
{
    QString mountpoint = RbSettings::value(RbSettings::Mountpoint).toString();
    // show dialog with error if mount point is wrong
    if(!QFileInfo(mountpoint).isDir()) {
        logger->addItem(tr("Mount point is wrong!"),LOGERROR);
        logger->setFinished();
        return true;
    }
    // Bootloader
    if(RbSettings::value(RbSettings::CurBootloaderMethod) != "none")
    {
        m_error = false;
        m_installed = false;
        m_auto = true;
        if(!installBootloaderAuto()) {
            logger->setFinished();
            return true;
        }
        else
        {
            // wait for boot loader installation finished
            while(!m_installed)
                QApplication::processEvents();
        }
        m_auto = false;
        if(m_error) return true;
        logger->setRunning();
    }

    // Rockbox
    m_error = false;
    m_installed = false;
    if(!installAuto())
        return true;
    else
    {
       // wait for installation finished
       while(!m_installed)
          QApplication::processEvents();
    }

    installBootloaderPost(false);
    return false;
}

void RbUtilQt::installdone(bool error)
{
    qDebug() << "install done";
    m_installed = true;
    m_error = error;
}

void RbUtilQt::installBtn()
{
    if(chkConfig(true)) return;
    install();
}

bool RbUtilQt::installAuto()
{
    QString file = QString("%1/%2/rockbox-%3-%4.zip")
            .arg(RbSettings::value(RbSettings::ReleaseUrl).toString(),
                versmap.value("rel_rev"),
                RbSettings::value(RbSettings::CurBuildserverModel).toString(),
                versmap.value("rel_rev"));
    buildInfo.open();
    QSettings info(buildInfo.fileName(), QSettings::IniFormat, this);
    buildInfo.close();

    // check installed Version and Target
    QString rbVersion = Detect::installedVersion(RbSettings::value(RbSettings::Mountpoint).toString());
    QString warning = Detect::check(false);

    if(!warning.isEmpty())
    {
        if(QMessageBox::warning(this, tr("Really continue?"), warning,
            QMessageBox::Ok | QMessageBox::Abort, QMessageBox::Abort) == QMessageBox::Abort)
        {
            logger->addItem(tr("Aborted!"), LOGERROR);
            logger->setFinished();
            return false;
        }
    }

    // check version
    if(rbVersion != "")
    {
        if(QMessageBox::question(this, tr("Installed Rockbox detected"),
           tr("Rockbox installation detected. Do you want to backup first?"),
           QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            logger->addItem(tr("Starting backup..."),LOGINFO);
            QString backupName = RbSettings::value(RbSettings::Mountpoint).toString()
                + "/.backup/rockbox-backup-" + rbVersion + ".zip";

            //! create dir, if it doesnt exist
            QFileInfo backupFile(backupName);
            if(!QDir(backupFile.path()).exists())
            {
                QDir a;
                a.mkpath(backupFile.path());
            }

            //! create backup
            RbZip backup;
            connect(&backup,SIGNAL(zipProgress(int,int)),logger, SLOT(setProgress(int,int)));
            if(backup.createZip(backupName,
                RbSettings::value(RbSettings::Mountpoint).toString() + "/.rockbox") == Zip::Ok)
            {
                logger->addItem(tr("Backup successful"),LOGOK);
            }
            else
            {
                logger->addItem(tr("Backup failed!"),LOGERROR);
                logger->setFinished();
                return false;
            }
        }
    }

    //! install current build
    ZipInstaller* installer = new ZipInstaller(this);
    installer->setUrl(file);
    installer->setLogSection("Rockbox (Base)");
    installer->setLogVersion(versmap.value("rel_rev"));
    if(!RbSettings::value(RbSettings::CacheDisabled).toBool())
        installer->setCache(true);
    installer->setMountPoint(RbSettings::value(RbSettings::Mountpoint).toString());

    connect(installer, SIGNAL(done(bool)), this, SLOT(installdone(bool)));
    connect(installer, SIGNAL(logItem(QString, int)), logger, SLOT(addItem(QString, int)));
    connect(installer, SIGNAL(logProgress(int, int)), logger, SLOT(setProgress(int, int)));
    connect(installer, SIGNAL(done(bool)), logger, SLOT(setFinished()));
    connect(logger, SIGNAL(aborted()), installer, SLOT(abort()));
    installer->install();
    return true;
}


void RbUtilQt::install()
{
    Install *installWindow = new Install(this);

    buildInfo.open();
    QSettings info(buildInfo.fileName(), QSettings::IniFormat, this);
    buildInfo.close();
    installWindow->setVersionStrings(versmap);

    installWindow->show();
}

bool RbUtilQt::installBootloaderAuto()
{
    installBootloader();
    return !m_error;
}

void RbUtilQt::installBootloaderBtn()
{
    if(chkConfig(true)) return;
    if(QMessageBox::question(this, tr("Confirm Installation"),
           tr("Do you really want to install the Bootloader?"),
              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    // create logger
    logger = new ProgressLoggerGui(this);
    logger->show();
    installBootloader();
}

void RbUtilQt::installBootloader()
{
    QString platform = RbSettings::value(RbSettings::Platform).toString();
    QString backupDestination = "";
    m_error = false;

    // create installer
    BootloaderInstallBase *bl;
    QString type = RbSettings::value(RbSettings::CurBootloaderMethod).toString();
    if(type == "mi4") {
        bl = new BootloaderInstallMi4(this);
    }
    else if(type == "hex") {
        bl = new BootloaderInstallHex(this);
    }
    else if(type == "sansa") {
        bl = new BootloaderInstallSansa(this);
    }
    else if(type == "ipod") {
        bl = new BootloaderInstallIpod(this);
    }
    else if(type == "file") {
        bl = new BootloaderInstallFile(this);
    }
    else {
        logger->addItem(tr("No install method known."), LOGERROR);
        logger->setFinished();
        return;
    }

    // set bootloader filename. Do this now as installed() needs it.
    QString blfile;
    blfile = RbSettings::value(RbSettings::Mountpoint).toString()
            + RbSettings::value(RbSettings::CurBootloaderFile).toString();
    // special case for H10 pure: this player can have a different
    // bootloader file filename. This is handled here to keep the install
    // class clean, though having it here is also not the nicest solution.
    if(RbSettings::value(RbSettings::Platform).toString() == "h10_ums"
        || RbSettings::value(RbSettings::Platform) == "h10_mtp") {
        if(resolvePathCase(blfile).isEmpty())
            blfile = RbSettings::value(RbSettings::Mountpoint).toString()
                + RbSettings::value(RbSettings::CurBootloaderName).toString()
                    .replace("H10", "H10EMP", Qt::CaseInsensitive);
    }
    bl->setBlFile(blfile);
    QUrl url(RbSettings::value(RbSettings::BootloaderUrl).toString()
            + RbSettings::value(RbSettings::CurBootloaderName).toString());
    bl->setBlUrl(url);
    bl->setLogfile(RbSettings::value(RbSettings::Mountpoint).toString()
                    + "/.rockbox/rbutil.log");

    if(bl->installed() == BootloaderInstallBase::BootloaderRockbox) {
        if(QMessageBox::question(this, tr("Bootloader detected"),
            tr("Bootloader already installed. Do you want to reinstall the bootloader?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
                if(m_auto) {
                    // keep logger open for auto installs.
                    // don't consider abort as error in auto-mode.
                    logger->addItem(tr("Bootloader installation skipped"), LOGINFO);
                    installBootloaderPost(false);
                }
                else {
                    logger->close();
                    installBootloaderPost(true);
                }
                return;
        }
    }
    else if(bl->installed() == BootloaderInstallBase::BootloaderOther
        && bl->capabilities() & BootloaderInstallBase::Backup)
    {
        QString targetFolder = RbSettings::value(RbSettings::CurPlatformName).toString()
                                + " Firmware Backup";
        // remove invalid character(s)
        targetFolder.remove(QRegExp("[:/]"));
        if(QMessageBox::question(this, tr("Create Bootloader backup"),
            tr("You can create a backup of the original bootloader "
               "file. Press \"Yes\" to select an output folder on your "
               "computer to save the file to. The file will get placed "
               "in a new folder \"%1\" created below the selected folder.\n"
               "Press \"No\" to skip this step.").arg(targetFolder),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            BrowseDirtree tree(this, tr("Browse backup folder"));
            tree.setDir(QDir::home());
            tree.exec();

            backupDestination = tree.getSelected() + "/" + targetFolder;
            qDebug() << backupDestination;
            // backup needs to be done after the logger has been set up.
        }
    }

    if(bl->capabilities() & BootloaderInstallBase::NeedsOf)
    {
        int ret;
        ret = QMessageBox::information(this, tr("Prerequisites"),
            bl->ofHint(),QMessageBox::Ok | QMessageBox::Abort);
        if(ret != QMessageBox::Ok) {
            // consider aborting an error to close window / abort automatic
            // installation.
            m_error = true;
            logger->addItem(tr("Bootloader installation aborted"), LOGINFO);
            logger->setFinished();
            return;
        }
        // open dialog to browse to of file
        QString offile;
        offile = QFileDialog::getOpenFileName(this,
                tr("Select firmware file"), QDir::homePath());
        if(!QFileInfo(offile).isReadable()) {
            logger->addItem(tr("Error opening firmware file"), LOGERROR);
            logger->setFinished();
            m_error = true;
            return;
        }
        bl->setOfFile(offile);
    }

    // the bootloader install class does NOT use any GUI stuff.
    // All messages are passed via signals.
    connect(bl, SIGNAL(done(bool)), logger, SLOT(setFinished()));
    connect(bl, SIGNAL(done(bool)), this, SLOT(installBootloaderPost(bool)));
    connect(bl, SIGNAL(logItem(QString, int)), logger, SLOT(addItem(QString, int)));
    connect(bl, SIGNAL(logProgress(int, int)), logger, SLOT(setProgress(int, int)));

    // start install.
    if(!backupDestination.isEmpty()) {
        if(!bl->backup(backupDestination)) {
            if(QMessageBox::warning(this, tr("Backup error"),
                    tr("Could not create backup file. Continue?"),
                    QMessageBox::No | QMessageBox::Yes)
                == QMessageBox::No) {
                logger->setFinished();
                return;
            }
        }
    }
    bl->install();
}

void RbUtilQt::installBootloaderPost(bool error)
{
    qDebug() << __func__ << error;
    // if an error occured don't perform post install steps.
    if(error) {
        m_error = true;
        return;
    }
    else
        m_error = false;

    m_installed = true;
    // end here if automated install
    if(m_auto)
        return;

    QString msg = BootloaderInstallBase::postinstallHints(
                    RbSettings::value(RbSettings::Platform).toString());
    if(!msg.isEmpty()) {
        QMessageBox::information(this, tr("Manual steps required"), msg);
        logger->close();
    }

}

void RbUtilQt::installFontsBtn()
{
    if(chkConfig(true)) return;
    if(QMessageBox::question(this, tr("Confirm Installation"),
           tr("Do you really want to install the fonts package?"),
              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    // create logger
    logger = new ProgressLoggerGui(this);
    logger->show();
    installFonts();
}

bool RbUtilQt::installFontsAuto()
{
    installFonts();

    return !m_error;
}

void RbUtilQt::installFonts()
{
    // create zip installer
    installer = new ZipInstaller(this);

    installer->setUrl(RbSettings::value(RbSettings::FontUrl).toString());
    installer->setLogSection("Fonts");
    installer->setLogVersion(versmap.value("arch_date"));
    installer->setMountPoint(RbSettings::value(RbSettings::Mountpoint).toString());
    if(!RbSettings::value(RbSettings::CacheDisabled).toBool())
        installer->setCache(true);

    connect(installer, SIGNAL(done(bool)), this, SLOT(installdone(bool)));
    connect(installer, SIGNAL(logItem(QString, int)), logger, SLOT(addItem(QString, int)));
    connect(installer, SIGNAL(logProgress(int, int)), logger, SLOT(setProgress(int, int)));
    connect(installer, SIGNAL(done(bool)), logger, SLOT(setFinished()));
    connect(logger, SIGNAL(aborted()), installer, SLOT(abort()));
    installer->install();
}


void RbUtilQt::installVoice()
{
    if(chkConfig(true)) return;

    if(m_gotInfo == false)
    {
       QMessageBox::warning(this, tr("Warning"),
       tr("The Application is still downloading Information about new Builds."
          " Please try again shortly."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirm Installation"),
       tr("Do you really want to install the voice file?"),
       QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    // create logger
    logger = new ProgressLoggerGui(this);
    logger->show();

    // create zip installer
    installer = new ZipInstaller(this);

    QString voiceurl = RbSettings::value(RbSettings::VoiceUrl).toString();

    voiceurl += RbSettings::value(RbSettings::CurConfigureModel).toString() + "-" +
        versmap.value("arch_date") + "-english.zip";
    qDebug() << voiceurl;

    installer->setUrl(voiceurl);
    installer->setLogSection("Voice");
    installer->setLogVersion(versmap.value("arch_date"));
    installer->setMountPoint(RbSettings::value(RbSettings::Mountpoint).toString());
    if(!RbSettings::value(RbSettings::CacheDisabled).toBool())
        installer->setCache(true);
    connect(installer, SIGNAL(logItem(QString, int)), logger, SLOT(addItem(QString, int)));
    connect(installer, SIGNAL(logProgress(int, int)), logger, SLOT(setProgress(int, int)));
    connect(installer, SIGNAL(done(bool)), logger, SLOT(setFinished()));
    connect(logger, SIGNAL(aborted()), installer, SLOT(abort()));
    installer->install();

}

void RbUtilQt::installDoomBtn()
{
    if(chkConfig(true)) return;
    if(!hasDoom()){
        QMessageBox::critical(this, tr("Error"),
            tr("Your device doesn't have a doom plugin. Aborting."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirm Installation"),
           tr("Do you really want to install the game addon files?"),
           QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    // create logger
    logger = new ProgressLoggerGui(this);
    logger->show();

    installDoom();
}
bool RbUtilQt::installDoomAuto()
{
    installDoom();
    return !m_error;
}

bool RbUtilQt::hasDoom()
{
    QFile doomrock(RbSettings::value(RbSettings::Mountpoint).toString()
                    +"/.rockbox/rocks/games/doom.rock");
    return doomrock.exists();
}

void RbUtilQt::installDoom()
{
    // create zip installer
    installer = new ZipInstaller(this);

    installer->setUrl(RbSettings::value(RbSettings::DoomUrl).toString());
    installer->setLogSection("Game Addons");
    installer->setLogVersion(versmap.value("arch_date"));
    installer->setMountPoint(RbSettings::value(RbSettings::Mountpoint).toString());
    if(!RbSettings::value(RbSettings::CacheDisabled).toBool())
        installer->setCache(true);
    connect(installer, SIGNAL(done(bool)), this, SLOT(installdone(bool)));
    connect(installer, SIGNAL(logItem(QString, int)), logger, SLOT(addItem(QString, int)));
    connect(installer, SIGNAL(logProgress(int, int)), logger, SLOT(setProgress(int, int)));
    connect(installer, SIGNAL(done(bool)), logger, SLOT(setFinished()));
    connect(logger, SIGNAL(aborted()), installer, SLOT(abort()));
    installer->install();

}

void RbUtilQt::installThemes()
{
    if(chkConfig(true)) return;
    ThemesInstallWindow* tw = new ThemesInstallWindow(this);
    tw->setModal(true);
    tw->show();
}

void RbUtilQt::createTalkFiles(void)
{
    if(chkConfig(true)) return;
    InstallTalkWindow *installWindow = new InstallTalkWindow(this);

    connect(installWindow, SIGNAL(settingsUpdated()), this, SLOT(downloadInfo()));
    connect(installWindow, SIGNAL(settingsUpdated()), this, SLOT(updateSettings()));
    installWindow->show();

}

void RbUtilQt::createVoiceFile(void)
{
    if(chkConfig(true)) return;
    CreateVoiceWindow *installWindow = new CreateVoiceWindow(this);
    
    connect(installWindow, SIGNAL(settingsUpdated()), this, SLOT(downloadInfo()));
    connect(installWindow, SIGNAL(settingsUpdated()), this, SLOT(updateSettings()));
    installWindow->show();
}

void RbUtilQt::uninstall(void)
{
    if(chkConfig(true)) return;
    UninstallWindow *uninstallWindow = new UninstallWindow(this);
    uninstallWindow->show();

}

void RbUtilQt::uninstallBootloader(void)
{
    if(chkConfig(true)) return;
    if(QMessageBox::question(this, tr("Confirm Uninstallation"),
           tr("Do you really want to uninstall the Bootloader?"),
           QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    // create logger
    ProgressLoggerGui* logger = new ProgressLoggerGui(this);
    logger->setProgressVisible(false);
    logger->show();

    QString platform = RbSettings::value(RbSettings::Platform).toString();

    // create installer
    BootloaderInstallBase *bl;
    QString type = RbSettings::value(RbSettings::CurBootloaderMethod).toString();
    if(type == "mi4") {
        bl = new BootloaderInstallMi4(this);
    }
    else if(type == "hex") {
        bl = new BootloaderInstallHex(this);
    }
    else if(type == "sansa") {
        bl = new BootloaderInstallSansa(this);
    }
    else if(type == "ipod") {
        bl = new BootloaderInstallIpod(this);
    }
    else if(type == "file") {
        bl = new BootloaderInstallFile(this);
    }
    else {
        logger->addItem(tr("No uninstall method known."), LOGERROR);
        logger->setFinished();
        return;
    }

    QString blfile = RbSettings::value(RbSettings::Mountpoint).toString()
        + RbSettings::value(RbSettings::CurBootloaderFile).toString();
    if(RbSettings::value(RbSettings::Platform).toString() == "h10_ums"
        || RbSettings::value(RbSettings::Platform).toString() == "h10_mtp") {
        if(resolvePathCase(blfile).isEmpty())
            blfile = RbSettings::value(RbSettings::Mountpoint).toString()
                + RbSettings::value(RbSettings::CurBootloaderName).toString()
                    .replace("H10", "H10EMP", Qt::CaseInsensitive);
    }
    bl->setBlFile(blfile);

    connect(bl, SIGNAL(logItem(QString, int)), logger, SLOT(addItem(QString, int)));
    connect(bl, SIGNAL(logProgress(int, int)), logger, SLOT(setProgress(int, int)));

    int result;
    result = bl->uninstall();

    logger->setFinished();

}


void RbUtilQt::downloadManual(void)
{
    if(chkConfig(true)) return;
    if(QMessageBox::question(this, tr("Confirm download"),
       tr("Do you really want to download the manual? The manual will be saved "
            "to the root folder of your player."),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    buildInfo.open();
    QSettings info(buildInfo.fileName(), QSettings::IniFormat, this);
    buildInfo.close();

    QString manual = RbSettings::value(RbSettings::CurManual).toString();

    QString date = (info.value("dailies/date").toString());

    QString manualurl;
    QString target;
    QString section;
    if(ui.radioPdf->isChecked()) {
        target = "/" + manual + ".pdf";
        section = "Manual (PDF)";
    }
    else {
        target = "/" + manual + "-" + date + "-html.zip";
        section = "Manual (HTML)";
    }
    manualurl = RbSettings::value(RbSettings::ManualUrl).toString() + "/" + target;
    qDebug() << "manualurl =" << manualurl;

    ProgressLoggerGui* logger = new ProgressLoggerGui(this);
    logger->show();
    installer = new ZipInstaller(this);
    installer->setMountPoint(RbSettings::value(RbSettings::Mountpoint).toString());
    if(!RbSettings::value(RbSettings::CacheDisabled).toBool())
        installer->setCache(true);
    installer->setLogSection(section);
    installer->setLogVersion(date);
    installer->setUrl(manualurl);
    installer->setUnzip(false);
    installer->setTarget(target);
    connect(installer, SIGNAL(logItem(QString, int)), logger, SLOT(addItem(QString, int)));
    connect(installer, SIGNAL(logProgress(int, int)), logger, SLOT(setProgress(int, int)));
    connect(installer, SIGNAL(done(bool)), logger, SLOT(setFinished()));
    connect(logger, SIGNAL(aborted()), installer, SLOT(abort()));
    installer->install();
}


void RbUtilQt::installPortable(void)
{
    if(QMessageBox::question(this, tr("Confirm installation"),
       tr("Do you really want to install Rockbox Utility to your player? "
        "After installation you can run it from the players hard drive."),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    ProgressLoggerGui* logger = new ProgressLoggerGui(this);
    logger->setProgressMax(0);
    logger->setProgressValue(0);
    logger->show();
    logger->addItem(tr("Installing Rockbox Utility"), LOGINFO);

    // check mountpoint
    if(!QFileInfo(RbSettings::value(RbSettings::Mountpoint).toString()).isDir()) {
        logger->addItem(tr("Mount point is wrong!"),LOGERROR);
        logger->setFinished();
        return;
    }

    // remove old files first.
    QFile::remove(RbSettings::value(RbSettings::Mountpoint).toString()
            + "/RockboxUtility.exe");
    QFile::remove(RbSettings::value(RbSettings::Mountpoint).toString()
            + "/RockboxUtility.ini");
    // copy currently running binary and currently used settings file
    if(!QFile::copy(qApp->applicationFilePath(),
            RbSettings::value(RbSettings::Mountpoint).toString()
            + "/RockboxUtility.exe")) {
        logger->addItem(tr("Error installing Rockbox Utility"), LOGERROR);
        logger->setFinished();
        return;
    }
    logger->addItem(tr("Installing user configuration"), LOGINFO);
    if(!QFile::copy(RbSettings::userSettingFilename(),
            RbSettings::value(RbSettings::Mountpoint).toString()
            + "/RockboxUtility.ini")) {
        logger->addItem(tr("Error installing user configuration"), LOGERROR);
        logger->setFinished();
        return;
    }
    logger->addItem(tr("Successfully installed Rockbox Utility."), LOGOK);
    logger->setFinished();
    logger->setProgressMax(1);
    logger->setProgressValue(1);

}


void RbUtilQt::updateInfo()
{
    qDebug() << "RbUtilQt::updateInfo()";

    QSettings log(RbSettings::value(RbSettings::Mountpoint).toString()
                    + "/.rockbox/rbutil.log", QSettings::IniFormat, this);
    QStringList groups = log.childGroups();
    QList<QTreeWidgetItem *> items;
    QTreeWidgetItem *w, *w2;
    QString min, max;
    int olditems = 0;

    // remove old list entries (if any)
    int l = ui.treeInfo->topLevelItemCount();
    while(l--) {
        QTreeWidgetItem *m;
        m = ui.treeInfo->takeTopLevelItem(l);
        // delete childs (single level deep, no recursion here)
        int n = m->childCount();
        while(n--)
            delete m->child(n);
    }
    // get and populate new items
    for(int a = 0; a < groups.size(); a++) {
        log.beginGroup(groups.at(a));
        QStringList keys = log.allKeys();
        w = new QTreeWidgetItem;
        w->setFlags(Qt::ItemIsEnabled);
        w->setText(0, groups.at(a));
        items.append(w);
        // get minimum and maximum version information so we can hilight old files
        min = max = log.value(keys.at(0)).toString();
        for(int b = 0; b < keys.size(); b++) {
            if(log.value(keys.at(b)).toString() > max)
                max = log.value(keys.at(b)).toString();
            if(log.value(keys.at(b)).toString() < min)
                min = log.value(keys.at(b)).toString();
        }

        for(int b = 0; b < keys.size(); b++) {
            QString file;
            file = RbSettings::value(RbSettings::Mountpoint).toString() + "/" + keys.at(b);
            if(QFileInfo(file).isDir())
                continue;
            w2 = new QTreeWidgetItem(w, QStringList() << "/"
                    + keys.at(b) << log.value(keys.at(b)).toString());
            if(log.value(keys.at(b)).toString() != max) {
                w2->setForeground(0, QBrush(QColor(255, 0, 0)));
                w2->setForeground(1, QBrush(QColor(255, 0, 0)));
                olditems++;
            }
            items.append(w2);
        }
        log.endGroup();
        if(min != max)
            w->setData(1, Qt::DisplayRole, QString("%1 / %2").arg(min, max));
        else
            w->setData(1, Qt::DisplayRole, max);
    }
    ui.treeInfo->insertTopLevelItems(0, items);
    ui.treeInfo->resizeColumnToContents(0);
}


QUrl RbUtilQt::proxy()
{
    if(RbSettings::value(RbSettings::ProxyType) == "manual")
        return QUrl(RbSettings::value(RbSettings::Proxy).toString());
    else if(RbSettings::value(RbSettings::ProxyType) == "system")
        return Detect::systemProxy();
    return QUrl("");
}


bool RbUtilQt::chkConfig(bool warn)
{
    bool error = false;
    if(RbSettings::value(RbSettings::Platform).toString().isEmpty()
        || RbSettings::value(RbSettings::Mountpoint).toString().isEmpty()
        || !QFileInfo(RbSettings::value(RbSettings::Mountpoint).toString()).isWritable()) {
        error = true;

        if(warn) QMessageBox::critical(this, tr("Configuration error"),
            tr("Your configuration is invalid. Please go to the configuration "
                "dialog and make sure the selected values are correct."));
    }
    return error;
}

