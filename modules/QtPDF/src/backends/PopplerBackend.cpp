/**
 * Copyright (C) 2011  Charlie Sharpsteen
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

// NOTE: `PopplerBackend.h` is included via `PDFBackend.h`
#include <PDFBackend.h>

// Document Class
// ==============
PopplerDocument::PopplerDocument(QString fileName):
  Super(fileName),
  _poppler_doc(Poppler::Document::load(fileName)),
  _doc_lock(new QMutex())
{
  _numPages = _poppler_doc->numPages();

  // **TODO:**
  //
  // _Make these configurable._
  _poppler_doc->setRenderBackend(Poppler::Document::SplashBackend);
  // Make things look pretty.
  _poppler_doc->setRenderHint(Poppler::Document::Antialiasing);
  _poppler_doc->setRenderHint(Poppler::Document::TextAntialiasing);
}

PopplerDocument::~PopplerDocument()
{
}

Page *PopplerDocument::page(int at){ return new PopplerPage(this, at); }


// Page Class
// ==========
PopplerPage::PopplerPage(PopplerDocument *parent, int at):
  Super(parent, at)
{
  _poppler_page = QSharedPointer<Poppler::Page>(reinterpret_cast<PopplerDocument *>(_parent)->_poppler_doc->page(at));
}

PopplerPage::~PopplerPage()
{
}

QSizeF PopplerPage::pageSizeF() { return _poppler_page->pageSizeF(); }

QImage PopplerPage::renderToImage(double xres, double yres, QRect render_box, bool cache)
{
  QImage renderedPage;

  // Rendering pages is not thread safe.
  QMutexLocker docLock(reinterpret_cast<PopplerDocument *>(_parent)->_doc_lock);
    if( render_box.isNull() ) {
      // A null QRect has a width and height of 0 --- we will tell Poppler to render the whole
      // page.
      renderedPage = _poppler_page->renderToImage(xres, yres);
    } else {
      renderedPage = _poppler_page->renderToImage(xres, yres,
          render_box.x(), render_box.y(), render_box.width(), render_box.height());
    }
  docLock.unlock();

  if( cache ) {
    PDFPageTile key(xres, yres, render_box, _n);
    // Don't cache a page if an entry already exists---it will cause the old
    // entry to be deleted which can invalidate some pointers.
    if( not _parent->pageCache().contains(key) ) {
      // Give the cache a copy so that it can take ownership. Use the size of
      // the image in bytes as the cost.
      _parent->pageCache().insert(key, new QImage(renderedPage.copy()), renderedPage.byteCount());
    }
  }

  return renderedPage;
}

QList<Poppler::Link *> PopplerPage::loadLinks()
{
  QList<Poppler::Link *> links;
  // Loading links is not thread safe.
  QMutexLocker docLock(reinterpret_cast<PopplerDocument *>(_parent)->_doc_lock);
    links = _poppler_page->links();
  docLock.unlock();

  return links;
}


// vim: set sw=2 ts=2 et
