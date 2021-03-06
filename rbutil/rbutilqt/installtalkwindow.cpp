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

#include "installtalkwindow.h"
#include "ui_installtalkfrm.h"

#include "browsedirtree.h"
#include "configure.h"
#include "rbsettings.h"

InstallTalkWindow::InstallTalkWindow(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
    talkcreator = new TalkFileCreator(this);

    connect(ui.buttonBrowse, SIGNAL(clicked()), this, SLOT(browseFolder()));
    connect(ui.change,SIGNAL(clicked()),this,SLOT(change()));

    ui.recursive->setChecked(true);
    ui.OverwriteTalk->setChecked(true);
    ui.StripExtensions->setChecked(true);
    
    updateSettings();
}

void InstallTalkWindow::browseFolder()
{
    BrowseDirtree browser(this);
    browser.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    if(QFileInfo(ui.lineTalkFolder->text()).isDir())
    {
        browser.setDir(ui.lineTalkFolder->text());
    }
    else
    {
        browser.setDir("/media"); // FIXME: This looks Linux specific
    }
    if(browser.exec() == QDialog::Accepted)
    {
        qDebug() << browser.getSelected();
        setTalkFolder(browser.getSelected());
    }
}

void InstallTalkWindow::setTalkFolder(QString folder)
{
    ui.lineTalkFolder->setText(folder);
}

void InstallTalkWindow::change()
{
    Config *cw = new Config(this,4);
    
    connect(cw, SIGNAL(settingsUpdated()), this, SLOT(updateSettings()));
    
    cw->show();
}

void InstallTalkWindow::accept()
{
    logger = new ProgressLoggerGui(this);
    
    connect(logger,SIGNAL(closed()),this,SLOT(close()));
    logger->show();
    

    QString folderToTalk = ui.lineTalkFolder->text();
     
    if(!QFileInfo(folderToTalk).isDir())
    {
        logger->addItem(tr("The Folder to Talk is wrong!"),LOGERROR);
        logger->setFinished();
        return;
    }

    RbSettings::setValue(RbSettings::LastTalkedFolder, folderToTalk);

    RbSettings::sync();

    talkcreator->setDir(QDir(folderToTalk));
    talkcreator->setMountPoint(RbSettings::value(RbSettings::Mountpoint).toString());
    
    talkcreator->setOverwriteTalk(ui.OverwriteTalk->isChecked());
    talkcreator->setRecursive(ui.recursive->isChecked());
    talkcreator->setStripExtensions(ui.StripExtensions->isChecked());
    talkcreator->setTalkFolders(ui.talkFolders->isChecked());
    talkcreator->setTalkFiles(ui.talkFiles->isChecked());
    
    talkcreator->createTalkFiles(logger);
}


void InstallTalkWindow::updateSettings(void)
{
    QString ttsName = RbSettings::value(RbSettings::Tts).toString();
    TTSBase* tts = TTSBase::getTTS(this,ttsName);
    if(tts->configOk())
        ui.labelTtsProfile->setText(tr("Selected TTS engine: <b>%1</b>")
            .arg(TTSBase::getTTSName(ttsName)));
    else
        ui.labelTtsProfile->setText(tr("Selected TTS engine: <b>%1</b>")
            .arg("Invalid TTS configuration!"));
    
    QString encoder = RbSettings::value(RbSettings::CurEncoder).toString();
    EncBase* enc = EncBase::getEncoder(this,encoder);
    if(enc != NULL) {
        if(enc->configOk())
            ui.labelEncProfile->setText(tr("Selected encoder: <b>%1</b>")
                .arg(EncBase::getEncoderName(encoder)));
        else
            ui.labelEncProfile->setText(tr("Selected encoder: <b>%1</b>")
                .arg("Invalid encoder configuration!"));
    }
    else
        ui.labelEncProfile->setText(tr("Selected encoder: <b>%1</b>")
            .arg("Invalid encoder configuration!"));

    setTalkFolder(RbSettings::value(RbSettings::LastTalkedFolder).toString());
    emit settingsUpdated();
}

