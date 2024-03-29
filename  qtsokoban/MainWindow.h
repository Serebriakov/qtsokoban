/*
 *  ksokoban - a Sokoban game for KDE
 *  Copyright (C) 1998  Anders Widell  <d95-awi@nada.kth.se>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QThread>
#include <QMainWindow>
#include "Bookmark.h"
#include "InternalCollections.h"

class QMenuBar;
class PlayField;
class QMenu;
class QFocusEvent;
class QDragEnterEvent;
class QDropEvent;
class LevelCollection;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();
  ~MainWindow();

  void openURL(QString _url);

public slots:
  void changeCollection(int id);
  void updateAnimMenu(int id);
  void setBookmark(int id);
  void goToBookmark(int id);
  void PlayMovie();
  void loadLevels();
  void LevelMovie();
  void MovieEnd();
  void about();

protected:
  void focusInEvent(QFocusEvent*);
  void createCollectionMenu();
  virtual void dragEnterEvent(QDragEnterEvent*);
  virtual void dropEvent(QDropEvent*);

private:
  InternalCollections internalCollections_;
  LevelCollection *externalCollection_;
  PlayField       *playField_;
  Bookmark        *bookmarks_[10];
  int              currentCollection_;


  QMenu      *game_;
  QMenu      *collection_;
  QMenu      *animation_;
  QMenu      *bookmarkMenu_;
  QMenu      *setBM_;
  QMenu      *goToBM_;
  QMenu      *help_;
  QAction *aboutQtAct;
  QAction *aboutAct;
  int              checkedCollection_;
  int              checkedAnim_;

  void updateBookmark(int num);

};

extern MainWindow *widget;

#endif  /* MAINWINDOW_H */
