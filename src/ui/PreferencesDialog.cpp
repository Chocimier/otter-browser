#include "PreferencesDialog.h"
#include "OptionDelegate.h"
#include "OptionWidget.h"
#include "../core/FileSystemCompleterModel.h"
#include "../core/SettingsManager.h"
#include "../core/SearchesManager.h"

#include "ui_PreferencesDialog.h"

#include <QtWidgets/QCompleter>
#include <QtWidgets/QFileDialog>

namespace Otter
{

PreferencesDialog::PreferencesDialog(const QString &section, QWidget *parent) : QDialog(parent),
	m_ui(new Ui::PreferencesDialog)
{
	m_ui->setupUi(this);

	if (section == "content")
	{
		m_ui->tabWidget->setCurrentIndex(1);
	}
	else if (section == "privacy")
	{
		m_ui->tabWidget->setCurrentIndex(2);
	}
	else if (section == "search")
	{
		m_ui->tabWidget->setCurrentIndex(3);
	}
	else if (section == "advanced")
	{
		m_ui->tabWidget->setCurrentIndex(4);
	}
	else
	{
		m_ui->tabWidget->setCurrentIndex(0);
	}

	m_ui->downloadsLineEdit->setCompleter(new QCompleter(new FileSystemCompleterModel(this), this));
	m_ui->downloadsLineEdit->setText(SettingsManager::getValue("Paths/Downloads").toString());
	m_ui->alwaysAskCheckBox->setChecked(SettingsManager::getValue("Browser/AlwaysAskWhereToSaveDownload").toBool());
	m_ui->tabsInsteadOfWindowsCheckBox->setChecked(SettingsManager::getValue("Browser/OpenLinksInNewTab").toBool());
	m_ui->openNextToActiveheckBox->setChecked(SettingsManager::getValue("Tabs/OpenNextToActive").toBool());

	m_ui->defaultZoomSpinBox->setValue(SettingsManager::getValue("Content/DefaultZoom").toInt());
	m_ui->zoomTextOnlyCheckBox->setChecked(SettingsManager::getValue("Content/ZoomTextOnly").toBool());
	m_ui->proportionalFontSizeSpinBox->setValue(SettingsManager::getValue("Content/DefaultFontSize").toInt());
	m_ui->fixedFontSizeSpinBox->setValue(SettingsManager::getValue("Content/DefaultFixedFontSize").toInt());
	m_ui->minimumFontSizeSpinBox->setValue(SettingsManager::getValue("Content/MinimumFontSize").toInt());

	QStringList fonts;
	fonts << "StandardFont" << "FixedFont" << "SerifFont" << "SansSerifFont" << "CursiveFont" << "FantasyFont";

	QStringList fontCategories;
	fontCategories << tr("Standard Font") << tr("Fixed Font") << tr("Serif Font") << tr("Sans Serif Font") << tr("Cursive Font") << tr("Fantasy Font");

	OptionDelegate *fontsDelegate = new OptionDelegate(true, this);

	m_ui->fontsWidget->setRowCount(fonts.count());
	m_ui->fontsWidget->setItemDelegateForColumn(1, fontsDelegate);

	for (int i = 0; i < fonts.count(); ++i)
	{
		const QString family = SettingsManager::getValue("Content/" + fonts.at(i)).toString();
		QTableWidgetItem *familyItem = new QTableWidgetItem(family);
		familyItem->setData(Qt::UserRole, "Content/" + fonts.at(i));
		familyItem->setData((Qt::UserRole + 1), "font");

		QTableWidgetItem *previewItem = new QTableWidgetItem(tr("The quick brown fox jumps over the lazy dog"));
		previewItem->setFont(QFont(family));

		m_ui->fontsWidget->setItem(i, 0, new QTableWidgetItem(fontCategories.at(i)));
		m_ui->fontsWidget->setItem(i, 1, familyItem);
		m_ui->fontsWidget->setItem(i, 2, previewItem);
	}

	QStringList colors;
	colors << "BackgroundColor" << "TextColor" << "LinkColor" << "VisitedLinkColor";

	QStringList colorTypes;
	colorTypes << tr("Background Color") << tr("Text Color") << tr("Link Color") << tr("Visited Link Color");

	OptionDelegate *colorsDelegate = new OptionDelegate(true, this);

	m_ui->colorsWidget->setRowCount(colors.count());
	m_ui->colorsWidget->setItemDelegateForColumn(1, colorsDelegate);

	for (int i = 0; i < colors.count(); ++i)
	{
		const QString color = SettingsManager::getValue("Content/" + colors.at(i)).toString();
		QTableWidgetItem *previewItem = new QTableWidgetItem(color);
		previewItem->setBackgroundColor(QColor(color));
		previewItem->setTextColor(Qt::transparent);
		previewItem->setData(Qt::UserRole, "Content/" + colors.at(i));
		previewItem->setData((Qt::UserRole + 1), "color");

		m_ui->colorsWidget->setItem(i, 0, new QTableWidgetItem(colorTypes.at(i)));
		m_ui->colorsWidget->setItem(i, 1, previewItem);
	}

	m_ui->privateModeCheckBox->setChecked(SettingsManager::getValue("Browser/PrivateMode").toBool());
	m_ui->historyWidget->setDisabled(m_ui->privateModeCheckBox->isChecked());
	m_ui->rememberDownloadsHistoryCheckBox->setChecked(SettingsManager::getValue("Browser/RememberDownloads").toBool());
	m_ui->acceptCookiesCheckBox->setChecked(SettingsManager::getValue("Browser/EnableCookies").toBool());

	const QStringList engines = SearchesManager::getEngines();

	for (int i = 0; i < engines.count(); ++i)
	{
		SearchInformation *engine = SearchesManager::getEngine(engines.at(i));

		if (engine)
		{
			QTableWidgetItem *engineItem = new QTableWidgetItem(engine->icon, engine->title);
			engineItem->setToolTip(engine->description);

			const int row = m_ui->searchWidget->rowCount();

			m_ui->searchWidget->setRowCount(row + 1);
			m_ui->searchWidget->setItem(row, 0, engineItem);
			m_ui->searchWidget->setItem(row, 1, new QTableWidgetItem(engine->shortcut));
		}
	}

	m_ui->searchWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

	connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(save()));
	connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
	connect(m_ui->downloadsBrowseButton, SIGNAL(clicked()), this, SLOT(browseDownloadsPath()));
	connect(m_ui->fontsWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(currentFontChanged(int,int,int,int)));
	connect(fontsDelegate, SIGNAL(commitData(QWidget*)), this, SLOT(fontChanged(QWidget*)));
	connect(m_ui->colorsWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(currentColorChanged(int,int,int,int)));
	connect(colorsDelegate, SIGNAL(commitData(QWidget*)), this, SLOT(colorChanged(QWidget*)));
	connect(m_ui->privateModeCheckBox, SIGNAL(toggled(bool)), m_ui->historyWidget, SLOT(setDisabled(bool)));
	connect(m_ui->searchFilterLineEdit, SIGNAL(textChanged(QString)), this, SLOT(filterSearch(QString)));
	connect(m_ui->searchWidget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(currentSearchChanged(int)));
	connect(m_ui->removeSearchButton, SIGNAL(clicked()), this, SLOT(removeSearch()));
	connect(m_ui->moveDownSearchButton, SIGNAL(clicked()), this, SLOT(moveDownSearch()));
	connect(m_ui->moveUpSearchButton, SIGNAL(clicked()), this, SLOT(moveUpSearch()));
}

PreferencesDialog::~PreferencesDialog()
{
	delete m_ui;
}

void PreferencesDialog::changeEvent(QEvent *event)
{
	QDialog::changeEvent(event);

	switch (event->type())
	{
		case QEvent::LanguageChange:
			m_ui->retranslateUi(this);

			break;
		default:
			break;
	}
}

void PreferencesDialog::browseDownloadsPath()
{
	const QString path = QFileDialog::getExistingDirectory(this, tr("Select Directory"), m_ui->downloadsLineEdit->text());

	if (!path.isEmpty())
	{
		m_ui->downloadsLineEdit->setText(path);
	}
}

void PreferencesDialog::currentFontChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
	Q_UNUSED(currentColumn)
	Q_UNUSED(previousColumn)

	QTableWidgetItem *previousItem = m_ui->fontsWidget->item(previousRow, 1);

	m_ui->fontsWidget->closePersistentEditor(previousItem);

	if (currentRow >= 0 && currentRow < m_ui->fontsWidget->rowCount())
	{
		m_ui->fontsWidget->openPersistentEditor(m_ui->fontsWidget->item(currentRow, 1));
	}
}

void PreferencesDialog::fontChanged(QWidget *editor)
{
	OptionWidget *widget = qobject_cast<OptionWidget*>(editor);

	if (widget && widget->getIndex().row() >= 0 && widget->getIndex().row() < m_ui->fontsWidget->rowCount())
	{
		m_ui->fontsWidget->item(widget->getIndex().row(), 1)->setText(m_ui->fontsWidget->item(widget->getIndex().row(), 1)->data(Qt::EditRole).toString());
		m_ui->fontsWidget->item(widget->getIndex().row(), 2)->setFont(QFont(widget->getValue().toString()));
	}
}

void PreferencesDialog::currentColorChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
	Q_UNUSED(currentColumn)
	Q_UNUSED(previousColumn)

	QTableWidgetItem *previousItem = m_ui->colorsWidget->item(previousRow, 1);

	m_ui->colorsWidget->closePersistentEditor(previousItem);

	if (currentRow >= 0 && currentRow < m_ui->colorsWidget->rowCount())
	{
		m_ui->colorsWidget->openPersistentEditor(m_ui->colorsWidget->item(currentRow, 1));
	}
}

void PreferencesDialog::colorChanged(QWidget *editor)
{
	OptionWidget *widget = qobject_cast<OptionWidget*>(editor);

	if (widget && widget->getIndex().row() >= 0 && widget->getIndex().row() < m_ui->colorsWidget->rowCount())
	{
		m_ui->colorsWidget->item(widget->getIndex().row(), 1)->setBackgroundColor(QColor(widget->getValue().toString()));
		m_ui->colorsWidget->item(widget->getIndex().row(), 1)->setData(Qt::EditRole, widget->getValue());
	}
}

void PreferencesDialog::filterSearch(const QString &filter)
{
	for (int i = 0; i < m_ui->searchWidget->rowCount(); ++i)
	{
		m_ui->searchWidget->setRowHidden(i, (!filter.isEmpty() && !m_ui->searchWidget->item(i, 0)->text().contains(filter, Qt::CaseInsensitive) && !m_ui->searchWidget->item(i, 1)->text().contains(filter, Qt::CaseInsensitive)));
	}
}

void PreferencesDialog::currentSearchChanged(int currentRow)
{
	m_ui->removeSearchButton->setEnabled(currentRow >= 0 && currentRow < m_ui->searchWidget->rowCount());
	m_ui->moveDownSearchButton->setEnabled(currentRow >= 0 && m_ui->searchWidget->rowCount() > 1 && currentRow < (m_ui->searchWidget->rowCount() - 1));
	m_ui->moveUpSearchButton->setEnabled(currentRow > 0 && m_ui->searchWidget->rowCount() > 1 && currentRow);
}

void PreferencesDialog::removeSearch()
{
	m_ui->searchWidget->removeRow(m_ui->searchWidget->currentIndex().row());
}

void PreferencesDialog::moveSearch(bool up)
{
	const int sourceRow = m_ui->searchWidget->currentIndex().row();
	const int destinationRow = (up ? (sourceRow - 1) : (sourceRow + 1));
	QTableWidgetItem *engineItem = m_ui->searchWidget->takeItem(sourceRow, 0);
	QTableWidgetItem *shortcutItem = m_ui->searchWidget->takeItem(sourceRow, 1);

	m_ui->searchWidget->removeRow(sourceRow);
	m_ui->searchWidget->insertRow(destinationRow);
	m_ui->searchWidget->setItem(destinationRow, 0, engineItem);
	m_ui->searchWidget->setItem(destinationRow, 1, shortcutItem);
	m_ui->searchWidget->setCurrentCell(destinationRow, 0);
}

void PreferencesDialog::moveDownSearch()
{
	moveSearch(false);
}

void PreferencesDialog::moveUpSearch()
{
	moveSearch(true);
}

void PreferencesDialog::save()
{
	SettingsManager::setValue("Paths/Downloads", m_ui->downloadsLineEdit->text());
	SettingsManager::setValue("Browser/AlwaysAskWhereSaveFile", m_ui->alwaysAskCheckBox->isChecked());
	SettingsManager::setValue("Browser/OpenLinksInNewTab", m_ui->tabsInsteadOfWindowsCheckBox->isChecked());
	SettingsManager::setValue("Tabs/OpenNextToActive", m_ui->openNextToActiveheckBox->isChecked());

	SettingsManager::setValue("Content/DefaultZoom", m_ui->defaultZoomSpinBox->value());
	SettingsManager::setValue("Content/ZoomTextOnly", m_ui->zoomTextOnlyCheckBox->isChecked());
	SettingsManager::setValue("Content/DefaultFontSize", m_ui->proportionalFontSizeSpinBox->value());
	SettingsManager::setValue("Content/DefaultFixedFontSize", m_ui->fixedFontSizeSpinBox->value());
	SettingsManager::setValue("Content/MinimumFontSize", m_ui->minimumFontSizeSpinBox->value());

	for (int i = 0; i < m_ui->fontsWidget->rowCount(); ++i)
	{
		SettingsManager::setValue(m_ui->fontsWidget->item(i, 1)->data(Qt::UserRole).toString() , m_ui->fontsWidget->item(i, 1)->data(Qt::DisplayRole));
	}

	for (int i = 0; i < m_ui->colorsWidget->rowCount(); ++i)
	{
		SettingsManager::setValue(m_ui->colorsWidget->item(i, 1)->data(Qt::UserRole).toString() , m_ui->colorsWidget->item(i, 1)->data(Qt::EditRole));
	}

	SettingsManager::setValue("Browser/PrivateMode", m_ui->privateModeCheckBox->isChecked());
	SettingsManager::setValue("Browser/RememberDownloads", m_ui->rememberDownloadsHistoryCheckBox->isChecked());
	SettingsManager::setValue("Browser/EnableCookies", m_ui->acceptCookiesCheckBox->isChecked());

	close();
}

}
