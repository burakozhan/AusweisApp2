/*
 * \copyright Copyright (c) 2016-2018 Governikus GmbH & Co. KG, Germany
 */

#include "PdfCreator.h"

#include <QDebug>
#include <QPagedPaintDevice>
#include <QPainter>
#include <QSvgRenderer>

using namespace governikus;


PdfCreator::PdfCreator(const QString& pFilename, const QString& pTitle, const QString& pHeadline, const QString& pContent)
	: mPdfWriter(pFilename)
	, mHeader()
	, mContent()
	, mFooter()
{
	mHeader.setUndoRedoEnabled(false);
	mContent.setUndoRedoEnabled(false);
	mFooter.setUndoRedoEnabled(false);

	qDebug() << "Use filename for PDF:" << pFilename;

	const QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMargins(20, 20, 20, 15), QPageLayout::Millimeter);
	mPdfWriter.setPageLayout(layout);
	mPdfWriter.setCreator(QCoreApplication::applicationName());
	mPdfWriter.setTitle(pTitle);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	mPdfWriter.setPdfVersion(QPagedPaintDevice::PdfVersion_A1b);
#endif

	createHeader(pTitle, pHeadline);
	createFooter();
	createContent(pContent);
}


void PdfCreator::createHeader(const QString& pTitle, const QString& pHeadline)
{
	const auto& header = QStringLiteral("<table cellspacing='5' cellpadding='5' width='100%'>"
										"<tr><td valign='middle'>"
										"<h1>%1 - %2</h1>"
										"<h3>%3"
										"<th rowspan='2' width='100'><img src='pdflogo' width='100' height='100'></th>"
										"</h3>"
										"</td></tr>"
										"<tr><td valign='middle'>"
										"<br><h3>%4</h3>"
										"</td></tr>"
										"</table>").arg(
			pTitle,
			QCoreApplication::applicationName(),
			tr("AusweisApp2 is a product of Governikus GmbH & Co. KG - on behalf of the Federal Ministry of the Interior, Building and Community."),
			pHeadline);

	QSvgRenderer renderer(QStringLiteral(":/images/npa.svg"));
	QImage image(768, 768, QImage::Format_RGB32);
	image.fill(0x00FFFFFF);
	QPainter imagePainter(&image);
	renderer.render(&imagePainter);

	mHeader.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("pdflogo")), image);
	mHeader.setHtml(header);
}


void PdfCreator::createContent(const QString& pContent)
{
	mContent.setHtml(pContent);
}


void PdfCreator::createFooter()
{
	const auto& footer = QStringLiteral("<h3>%1</h3>").arg(
			tr("For further information, please see <a href='https://www.ausweisapp.bund.de/'>https://www.ausweisapp.bund.de/</a>"));

	mFooter.setHtml(footer);
}


int qt_defaultDpi();
bool PdfCreator::save()
{
	mPdfWriter.setResolution(qt_defaultDpi());
	const QRect pageArea(mPdfWriter.pageLayout().paintRectPixels(mPdfWriter.resolution()));

	QPainter painter(&mPdfWriter);
	if (!painter.isActive())
	{
		qCritical() << "Cannot paint into pdf file. Check file system permissions!";
		return false;
	}

	mHeader.setPageSize(pageArea.size());
	mFooter.setPageSize(pageArea.size());
	const qreal headerHeight = mHeader.size().height();
	const QSizeF contentMaxPageSize(pageArea.width(), pageArea.height() - headerHeight - mFooter.size().height());
	mContent.setPageSize(contentMaxPageSize);

	const QRect contentRect = QRect(QPoint(0, 0), mContent.size().toSize());
	QRect currentRect = QRect(QPoint(0, 0), contentMaxPageSize.toSize());
	while (currentRect.intersects(contentRect))
	{
		painter.resetTransform();

		mHeader.drawContents(&painter);
		painter.translate(0, headerHeight);

		painter.save();
		painter.translate(0, -currentRect.y());
		mContent.drawContents(&painter, currentRect);
		painter.restore();
		painter.translate(0, currentRect.height());

		mFooter.drawContents(&painter);

		currentRect.translate(0, currentRect.height());
		if (currentRect.intersects(contentRect))
		{
			mPdfWriter.newPage();
		}
	}

	return true;
}
