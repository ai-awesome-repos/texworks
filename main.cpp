#include "QTeXApp.h"
#include "TeXDocument.h"

int main(int argc, char *argv[])
{
	QTeXApp app(argc, argv);

	TeXDocument *mainWin = new TeXDocument;
	mainWin->show();

	return app.exec();
}