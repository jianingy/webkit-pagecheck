#include <QApplication>
#include <QtWebKit>
#include <QtGui>
#include <QPrinter>
#include <QTimer>
#include <QByteArray>
#include <QNetworkRequest>
#include <iostream>
#include "PageCheck.hh"

#ifdef STATIC_PLUGINS
  Q_IMPORT_PLUGIN(qjpeg)
#endif

QString
PageCheckPage::chooseFile(QWebFrame* /*frame*/, const QString& /*suggestedFile*/)
{
  return QString::null;
}

bool
PageCheckPage::javaScriptConfirm(QWebFrame* /*frame*/, const QString& /*msg*/)
{
  return true;
}

bool
PageCheckPage::javaScriptPrompt(QWebFrame* /*frame*/,
                           const QString& /*msg*/,
                           const QString& /*defaultValue*/,
                           QString* /*result*/)
{
  return true;
}

void
PageCheckPage::javaScriptConsoleMessage(const QString& /*message*/,
                                   int /*lineNumber*/,
                                   const QString& /*sourceID*/) 
{
  // noop
}

void
PageCheckPage::javaScriptAlert(QWebFrame* /*frame*/, const QString& /*msg*/)
{
  // noop
}

QString
PageCheckPage::userAgentForUrl(const QUrl& url) const
{

  if (!mUserAgent.isNull())
    return mUserAgent;

  return QWebPage::userAgentForUrl(url);
}

void
PageCheckPage::setUserAgent(const QString& userAgent)
{
  mUserAgent = userAgent;
}

void
PageCheckPage::setAttribute(QWebSettings::WebAttribute option,
                       const QString& value)
{

  if (value == "on")
    settings()->setAttribute(option, true);
  else if (value == "off")
    settings()->setAttribute(option, false);
  else
    (void)0; // TODO: ...
}

PageCheck::PageCheck(PageCheckPage* page, const QString& script, const QString& output, int delay) {
  mPage = page;
  mOutput = output;
  mDelay = delay;
  mScript = script;
  mSawInitialLayout = false;
  mSawDocumentComplete = false;
}

void
PageCheck::InitialLayoutCompleted()
{
  mSawInitialLayout = true;

  QWebFrame *mainFrame = mPage->mainFrame();
  QPainter painter;
  QFile file(mScript);
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream stream(&file);
  QString scriptText = stream.readAll();
//  std::cerr << mScript.toAscii().constData() << ": " << scriptText.toAscii().constData() << std::endl;
  mainFrame->evaluateJavaScript(scriptText);

  if (mSawInitialLayout && mSawDocumentComplete)
    TryDelayedRender();
}

void
PageCheck::DocumentComplete(bool /*ok*/)
{
  mSawDocumentComplete = true;

  if (mSawInitialLayout && mSawDocumentComplete)
    TryDelayedRender();
}

void
PageCheck::TryDelayedRender()
{

  if (mDelay > 0) {
    QTimer::singleShot(mDelay, this, SLOT(Delayed()));
    return;
  }

  check();
  QApplication::exit();
}

void
PageCheck::Timeout()
{
  check();
  QApplication::exit();
}

void
PageCheck::Delayed()
{
  check();
  QApplication::exit();
}

void
PageCheck::check()
{
  QWebFrame *mainFrame = mPage->mainFrame();
  std::cerr << mainFrame->toHtml().toAscii().constData() << std::endl;
}


int
main(int argc, char *argv[])
{
  int argDelay = 0;
  int argMinWidth = 1024;
  int argDefHeight = 768;
  int argMaxWait = 90000;

  if (argc < 2)
    exit(0);
  const char* argUrl = argv[1];
  const char* argScript = argv[2];
  const char* argUserStyle = NULL;
  const char* argIconDbPath = NULL;
  QString argOut("output.log");

  QApplication app(argc, argv, true);
  PageCheckPage page;

  QNetworkAccessManager::Operation method =
    QNetworkAccessManager::GetOperation;
  QByteArray body;
  QNetworkRequest req;

  req.setUrl( QUrl(argUrl) );

  PageCheck main(&page, argScript, argOut, argDelay);

  app.connect(&page,
    SIGNAL(loadFinished(bool)),
    &main,
    SLOT(DocumentComplete(bool)));

  app.connect(page.mainFrame(),
    SIGNAL(initialLayoutCompleted()),
    &main,
    SLOT(InitialLayoutCompleted()));
 
  if (argMaxWait > 0) {
    // TODO: Should this also register one for the application?
    QTimer::singleShot(argMaxWait, &main, SLOT(Timeout()));
  }

  if (argUserStyle != NULL)
    // TODO: does this need any syntax checking?
    page.settings()->setUserStyleSheetUrl( QUrl(argUserStyle) );

  if (argIconDbPath != NULL)
    // TODO: does this need any syntax checking?
    page.settings()->setIconDatabasePath(argUserStyle);

  // The documentation does not say, but it seems the mainFrame
  // will never change, so we can set this here. Otherwise we'd
  // have to set this in snapshot and trigger an update, which
  // is not currently possible (Qt 4.4.0) as far as I can tell.
  page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
  page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
  page.setViewportSize( QSize(argMinWidth, argDefHeight) );

  if (!body.isNull())
    page.mainFrame()->load(req, method, body);
  else
    page.mainFrame()->load(req, method);

  return app.exec();
}
