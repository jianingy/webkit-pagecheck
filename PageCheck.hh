#include <QtWebKit>

class PageCheckPage: public QWebPage {
  Q_OBJECT

public:
  void setAttribute(QWebSettings::WebAttribute option, const QString& value);
  void setUserAgent(const QString& userAgent);

protected:
  QString chooseFile(QWebFrame *frame, const QString& suggestedFile);
  void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
  bool javaScriptPrompt(QWebFrame* frame, const QString& msg, const QString& defaultValue, QString* result);
  void javaScriptAlert(QWebFrame* frame, const QString& msg);
  bool javaScriptConfirm(QWebFrame* frame, const QString& msg);
  QString userAgentForUrl(const QUrl& url) const;
  QString mUserAgent;
};

class PageCheck: public QObject {
  Q_OBJECT

public:

  PageCheck(PageCheckPage* page, const QString& script, const QString& output, int delay);

private slots:
  void DocumentComplete(bool ok);
  void InitialLayoutCompleted();
  void Timeout();
  void Delayed();

private:
  void TryDelayedRender();
  void check();
  bool mSawInitialLayout;
  bool mSawDocumentComplete;

protected:
  QString           mOutput;
  int               mDelay;
  PageCheckPage*    mPage;
  QString           mScript;
};
