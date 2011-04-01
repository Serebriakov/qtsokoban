/*
 *  ksokoban - a Sokoban game for KDE
 *  Copyright (C) 1998  Anders Widell  <awl@hem.passagen.se>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QKeySequence>
#include <assert.h>
#include <qstring.h>
#include <qframe.h>
#include <qiconset.h>
#include <Q3DragObject>
#include <QMessageBox>
#include <QSettings>

#include "MainWindow.h"
#include "PlayField.h"
#include "LevelCollection.h"
#include "StaticImage.h"

#define i18n(str) str
void
MainWindow::createCollectionMenu() {
  collection_ = new QMenu("collection menu", 0);
  collection_->setCheckable(true);

  connect(collection_, SIGNAL(activated(int)), this, SLOT(changeCollection(int)));

  for (int i=0; i<internalCollections_.collections(); i++) {
    collection_->insertItem(internalCollections_[i]->name(), i);
  }
  checkedCollection_ = 0;

  QSettings settings("Sokoban.ini", QSettings::IniFormat);

  int id = settings.value("settings/collection", 10).asInt();

  currentCollection_ = 0;
  for (int i=0; i<internalCollections_.collections(); i++) {
    if (internalCollections_[i]->id() == id) currentCollection_ = i;
  }

  changeCollection(currentCollection_);
}

void MainWindow::about()
 {

   QString translatedTextAbout = tr(
        "<h3>About QSokoban</h3>"
        "<p>QSokoban is a game porting to Windows from  KSokoban</p>"
		"<p>                         by <a href=\"mailto:jia.jacky@gmail.com\">Jacky Jia</a> for <a href=\"http://idk828.blog.sohu.com/\">IDK</a></p>"
		"<p>License:GPL (c) 1998-2001</p>"
		"<p>More info see <a href=\"http://hem.passagen.se/awl/ksokoban/\">http://hem.passagen.se/awl/ksokoban/</a></p>"
		"<p>Source code <a href=\"http://code.google.com/p/qtsokoban/\">http://code.google.com/p/qtsokoban/</a></p>"
		"<p> </p>"
		"<p> </p>"
		"<h3>Update</h3>"
		"<p>1.save move after compeleted level</p>"
		"<p>2.play moving movie</p>"
		);
   QMessageBox *qsokoban_about = new QMessageBox(QMessageBox::NoIcon,QObject::tr("About QSokoban"),translatedTextAbout);
   qsokoban_about->setIconPixmap(playField_->imageData_->idk());
   qsokoban_about->show();
 }


MainWindow::MainWindow() : QMainWindow(0), externalCollection_(0) {
  int i;
  QPixmap pixmap;

  setEraseColor(QColor(0,0,0));

  setWindowTitle(i18n("Qsokoban for IDK"));
  QSettings settings("Sokoban.ini", QSettings::IniFormat);
  int width = settings.value("Geometry/width", 750).asInt();
  int height = settings.value("Geometry/height", 562).asInt();
  resize(width, height);

  playField_ = new PlayField(this, "playfield");
  setCentralWidget(playField_);
  playField_->show();


  game_  = menuBar()->addMenu(i18n("&Game"));
  pixmap = NULL;//SmallIcon("fileopen");
  game_->insertItem(QIconSet(pixmap), i18n("&Load Levels..."), this, SLOT(loadLevels()));
  //pixmap = NULL;//SmallIcon("forward");
  game_->insertItem(QIconSet(pixmap), i18n("Level &Movie"), this, SLOT(LevelMovie()),Qt::Key_M);
  pixmap = NULL;//SmallIcon("forward");
  game_->insertItem(QIconSet(pixmap), i18n("&Next Level"), playField_, SLOT(nextLevel()), Qt::Key_N);
  pixmap = NULL;//SmallIcon("back");
  game_->insertItem(QIconSet(pixmap), i18n("&Previous Level"), playField_, SLOT(previousLevel()), Qt::Key_P);
  pixmap = NULL;//SmallIcon("reload");
  game_->insertItem(QIconSet(pixmap), i18n("Re&start Level"), playField_, SLOT(restartLevel()), Qt::Key_Escape);

  createCollectionMenu();
  game_->insertItem(i18n("&Level Collection"), collection_);

  pixmap = NULL;//SmallIcon("undo");
  game_->insertItem(QIconSet(pixmap), i18n("&Undo"), playField_, SLOT(undo()),QKeySequence( /*(KStdAccel::undo()).toString()*/Qt::CTRL+Qt::Key_Z));
  pixmap = NULL;//SmallIcon("redo");
  game_->insertItem(QIconSet(pixmap), i18n("&Redo"), playField_, SLOT(redo()), QKeySequence( /*(KStdAccel::redo()).toString()*/Qt::CTRL+Qt::SHIFT+Qt::Key_Z));
  game_->insertSeparator();
  pixmap = NULL;//SmallIcon("exit");
  game_->insertItem(QIconSet(pixmap), i18n("&Quit"), qApp, SLOT(closeAllWindows()), QKeySequence( /*(KStdAccel::quit()).toString()*/Qt::CTRL+Qt::Key_Q ));
  
  animation_ = menuBar()->addMenu(i18n("&Animation"));
  animation_->setCheckable(true);
  connect(animation_, SIGNAL(activated(int)), this, SLOT(updateAnimMenu(int)));
  connect(animation_, SIGNAL(activated(int)), playField_, SLOT(changeAnim(int)));
  animation_->insertItem(i18n("&Slow"), 3);
  animation_->insertItem(i18n("&Medium"), 2);
  animation_->insertItem(i18n("&Fast"), 1);
  animation_->insertItem(i18n("&Off"), 0);
  checkedAnim_ = playField_->animDelay();
  updateAnimMenu(checkedAnim_);
  
  bookmarkMenu_ = menuBar()->addMenu(i18n("&Bookmarks"));
  pixmap = NULL;//SmallIcon("bookmark_add");
  setBM_ = new QMenu("set bookmark menu", 0);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 1);
  setBM_->setAccel(Qt::CTRL+Qt::Key_1, 1);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 2);
  setBM_->setAccel(Qt::CTRL+Qt::Key_2, 2);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 3);
  setBM_->setAccel(Qt::CTRL+Qt::Key_3, 3);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 4);
  setBM_->setAccel(Qt::CTRL+Qt::Key_4, 4);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 5);
  setBM_->setAccel(Qt::CTRL+Qt::Key_5, 5);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 6);
  setBM_->setAccel(Qt::CTRL+Qt::Key_6, 6);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 7);
  setBM_->setAccel(Qt::CTRL+Qt::Key_7, 7);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 8);
  setBM_->setAccel(Qt::CTRL+Qt::Key_8, 8);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 9);
  setBM_->setAccel(Qt::CTRL+Qt::Key_9, 9);
  setBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 10);
  setBM_->setAccel(Qt::CTRL+Qt::Key_0, 10);
  connect(setBM_, SIGNAL(activated(int)), this, SLOT(setBookmark(int)));
  bookmarkMenu_->insertItem(i18n("&Set Bookmark"), setBM_);

  pixmap = NULL;//SmallIcon("bookmark");
  goToBM_ = new QMenu("go to bookmark menu", 0);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 1);
  goToBM_->setAccel(Qt::Key_1, 1);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 2);
  goToBM_->setAccel(Qt::Key_2, 2);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 3);
  goToBM_->setAccel(Qt::Key_3, 3);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 4);
  goToBM_->setAccel(Qt::Key_4, 4);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 5);
  goToBM_->setAccel(Qt::Key_5, 5);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 6);
  goToBM_->setAccel(Qt::Key_6, 6);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 7);
  goToBM_->setAccel(Qt::Key_7, 7);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 8);
  goToBM_->setAccel(Qt::Key_8, 8);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 9);
  goToBM_->setAccel(Qt::Key_9, 9);
  goToBM_->insertItem(QIconSet(pixmap), i18n("(unused)"), 10);
  goToBM_->setAccel(Qt::Key_0, 10);
  connect(goToBM_, SIGNAL(activated(int)), this, SLOT(goToBookmark(int)));
  bookmarkMenu_->insertItem(i18n("&Go to Bookmark"), goToBM_);


  help_ = menuBar()->addMenu(i18n("&Help"));

  aboutQtAct = new QAction("About &Qt", this);
  aboutQtAct->setStatusTip("Show the Qt library's About box");
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  
  aboutAct = new QAction("&About", this);
  aboutAct->setStatusTip("Show the application's About box");
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  help_->addAction(aboutAct);
  help_->addAction(aboutQtAct);

  for (i=1; i<=10; i++) {
    bookmarks_[i-1] = new Bookmark(i);
    updateBookmark(i);
  }

  setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
  QSettings settings("Sokoban.ini", QSettings::IniFormat);

  settings.setValue("Geometry/width", width());
  settings.setValue("Geometry/height", height());
  settings.setValue("settings/collection", internalCollections_[checkedCollection_]->id());

  settings.sync();
  for (int i=1; i<=10; i++) {
    delete bookmarks_[i-1];
  }


  delete externalCollection_;


  delete goToBM_;
  delete setBM_;
  delete bookmarkMenu_;
  delete animation_;
  delete collection_;
  delete game_;
  delete playField_;
}



void
MainWindow::focusInEvent(QFocusEvent *) {
  playField_->setFocus();
}

void
MainWindow::updateAnimMenu(int id) {
  animation_->setItemChecked(checkedAnim_, false);
  checkedAnim_ = id;
  animation_->setItemChecked(checkedAnim_, true);
}

void
MainWindow::updateBookmark(int num) {
  int col = internalCollections_.toInternalId(bookmarks_[num-1]->collection());
  int lev = bookmarks_[num-1]->level();
  int mov = bookmarks_[num-1]->moves();

  if (col < 0 || lev < 0) return;

  QString name;
  if (col >= 0 && col < internalCollections_.collections())
    name = internalCollections_[col]->name();
  else
    name = i18n("(invalid)");
  QString l;
  l.setNum(lev+1);
  name += " #" + l;
  l.setNum(mov);
  name += " (" + l + ")";

  setBM_->changeItem(num, name);
  goToBM_->changeItem(num, name);
}

void
MainWindow::setBookmark(int id) {
  assert(id >= 1 && id <= 10);
  playField_->setBookmark(bookmarks_[id-1]);
  updateBookmark(id);
}

void
MainWindow::goToBookmark(int id) {
  assert(id >= 1 && id <= 10);

  Bookmark *bm = bookmarks_[id-1];
  int collection = internalCollections_.toInternalId(bm->collection());
  int level = bm->level();

  if (collection < 0 || collection >= internalCollections_.collections()) return;
  LevelCollection* colPtr = internalCollections_[collection];
  if (colPtr == 0) return;
  if (level < 0 || level >= colPtr->noOfLevels()) return;
  if (level > colPtr->completedLevels()) return;

  playField_->setUpdatesEnabled(false);
  changeCollection(collection);
  playField_->setUpdatesEnabled(true);
  playField_->goToBookmark(bookmarks_[id-1]);
}

void
MainWindow::changeCollection(int id)
{
  collection_->setItemChecked(checkedCollection_, false);
  checkedCollection_ = id;
  collection_->setItemChecked(checkedCollection_, true);

  delete externalCollection_;
  externalCollection_ = 0;
  playField_->changeCollection(internalCollections_[id]);
}

void
MainWindow::loadLevels() {
 // KConfig *cfg=(KApplication::kApplication())->config();
 // cfg->setGroup("settings");
 // QString lastFile = cfg->readPathEntry("lastLevelFile");

  //KURL result = KFileDialog::getOpenURL(lastFile, "*", this, i18n("Load Levels From File"));
  //if (result.isEmpty()) return;

  //openURL(result);
}

void
MainWindow::LevelMovie() {
  game_->setEnabled(false);
  collection_->setEnabled(false);
  animation_->setEnabled(false);
  bookmarkMenu_->setEnabled(false);
  help_->setEnabled(false);
  PlayMovie();
  //if(!playthread)
  //  playthread = new MovieThread();
  //playthread->window = widget;

  //playthread->start();
  //playthread->wait();
  return;
}

void
MainWindow::openURL(QString _url) {
//  KConfig *cfg=(KApplication::kApplication())->config();

//   int namepos = _url.path().findRev('/') + 1; // NOTE: findRev can return -1
//   QString levelName = _url.path().mid(namepos);
  QString levelName = _url;

  QString levelFile;
/*  if (_url.isLocalFile()) {
    levelFile = _url.path();
  } else {
//     levelFile = locateLocal("appdata", "levels/" + levelName);
    if(!KIO::NetAccess::download( _url, levelFile ) )
	  return;
  }
*/
  LevelCollection *tmpCollection = new LevelCollection(levelFile, levelName);
//  KIO::NetAccess::removeTempFile(levelFile );

  if (tmpCollection->noOfLevels() < 1) {
    QMessageBox::warning(this, "sorry", i18n("No levels found in file"));
    delete tmpCollection;
    return;
  }
/*
  if (_url.isLocalFile()) {
	QSettings settings("Sokoban.ini", QSettings::IniFormat);
	settings.setValue("settings/lastLevelFile", _url.path());
  }
*/
  delete externalCollection_;
  externalCollection_ = tmpCollection;

  collection_->setItemChecked(checkedCollection_, false);
  playField_->changeCollection(externalCollection_);


}

void
MainWindow::dragEnterEvent(QDragEnterEvent* event) {
//  event->accept(KURLDrag::canDecode(event));
}

void
MainWindow::dropEvent(QDropEvent* event) {
/*
  KURL::List urls;
  if (KURLDrag::decode(event, urls)) {
     kdDebug() << "MainWindow:Handling QUriDrag..." << endl;
     if (urls.count() > 0) {
         const KURL &url = urls.first();
         openURL(url);
     }
  }
  */
}
void
MainWindow::PlayMovie(){
  if (!playField_->playHistory())
    MovieEnd();
}


void
MainWindow::MovieEnd(){
  game_->setEnabled(true);
  collection_->setEnabled(true);
  animation_->setEnabled(true);
  bookmarkMenu_->setEnabled(true);
  help_->setEnabled(true);
  return;
}