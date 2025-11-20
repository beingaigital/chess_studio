/*
    This file is part of Cute Chess.

    Copyright (C) 2008-2018 Cute Chess authors

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "positionsetupdlg.h"
#include "ui_positionsetupdlg.h"

#include <QClipboard>
#include <QColor>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QVariant>

namespace {

const QString kDefaultFen =
	QStringLiteral("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

bool isValidEnPassantSquare(const QString& square)
{
	if (square.size() != 2)
		return false;

	QChar file = square[0].toLower();
	QChar rank = square[1];
	return file >= QLatin1Char('a') && file <= QLatin1Char('h')
		&& (rank == QLatin1Char('3') || rank == QLatin1Char('6'));
}

bool isValidPieceSymbol(QChar symbol)
{
	static const QString allowed = QStringLiteral(".KQRBNPkqrbnp");
	return allowed.contains(symbol);
}

} // namespace

PositionSetupDialog::PositionState::PositionState()
{
	reset();
}

void PositionSetupDialog::PositionState::reset()
{
	clearBoard();
	whiteToMove = true;
	castleK = castleQ = castlek = castleq = true;
	enPassant = QStringLiteral("-");
	halfmoveClock = 0;
	fullmoveNumber = 1;
}

void PositionSetupDialog::PositionState::clearBoard()
{
	for (int row = 0; row < 8; ++row)
		for (int col = 0; col < 8; ++col)
			squares[row][col] = QLatin1Char('.');
}

QString PositionSetupDialog::PositionState::toFen() const
{
	QString fen;
	for (int row = 0; row < 8; ++row)
	{
		int empty = 0;
		for (int col = 0; col < 8; ++col)
		{
			const QChar symbol = squares[row][col];
			if (symbol == QLatin1Char('.'))
				empty++;
			else
			{
				if (empty > 0)
				{
					fen += QString::number(empty);
					empty = 0;
				}
				fen += symbol;
			}
		}
		if (empty > 0)
			fen += QString::number(empty);
		if (row != 7)
			fen += QLatin1Char('/');
	}

	fen += whiteToMove ? QStringLiteral(" w ") : QStringLiteral(" b ");

	QString castling;
	if (castleK)
		castling += QLatin1Char('K');
	if (castleQ)
		castling += QLatin1Char('Q');
	if (castlek)
		castling += QLatin1Char('k');
	if (castleq)
		castling += QLatin1Char('q');
	if (castling.isEmpty())
		castling = QStringLiteral("-");

	fen += castling;
	fen += QLatin1Char(' ');
	fen += enPassant.isEmpty() ? QStringLiteral("-") : enPassant;
	fen += QLatin1Char(' ');
	fen += QString::number(halfmoveClock);
	fen += QLatin1Char(' ');
	fen += QString::number(fullmoveNumber);

	return fen;
}

bool PositionSetupDialog::PositionState::fromFen(const QString& fen)
{
	PositionState copy;
	copy.clearBoard();

	const QString trimmed = fen.trimmed();
	const auto parts = trimmed.split(QLatin1Char(' '), Qt::SkipEmptyParts);
	if (parts.size() < 4)
		return false;

	const auto ranks = parts[0].split(QLatin1Char('/'));
	if (ranks.size() != 8)
		return false;

	for (int row = 0; row < 8; ++row)
	{
		const QString rank = ranks[row];
		int file = 0;
		for (const QChar ch : rank)
		{
			if (ch.isDigit())
			{
				file += ch.digitValue();
			}
			else if (isValidPieceSymbol(ch))
			{
				if (file >= 8)
					return false;
				copy.squares[row][file++] = ch;
			}
			else
				return false;
		}
		if (file != 8)
			return false;
	}

	if (parts[1].size() != 1)
		return false;
	if (parts[1] == QLatin1String("w"))
		copy.whiteToMove = true;
	else if (parts[1] == QLatin1String("b"))
		copy.whiteToMove = false;
	else
		return false;

	const QString castling = parts[2];
	copy.castleK = castling.contains(QLatin1Char('K'));
	copy.castleQ = castling.contains(QLatin1Char('Q'));
	copy.castlek = castling.contains(QLatin1Char('k'));
	copy.castleq = castling.contains(QLatin1Char('q'));
	if (castling == QLatin1String("-"))
		copy.castleK = copy.castleQ = copy.castlek = copy.castleq = false;

	QString enPassant = parts[3].toLower();
	if (enPassant == QLatin1String("-"))
		copy.enPassant = QStringLiteral("-");
	else if (isValidEnPassantSquare(enPassant))
		copy.enPassant = enPassant;
	else
		return false;

	if (parts.size() >= 5)
	{
		bool ok = false;
		const int halfmove = parts[4].toInt(&ok);
		if (!ok || halfmove < 0)
			return false;
		copy.halfmoveClock = halfmove;
	}
	else
		copy.halfmoveClock = 0;

	if (parts.size() >= 6)
	{
		bool ok = false;
		const int fullmove = parts[5].toInt(&ok);
		if (!ok || fullmove < 1)
			return false;
		copy.fullmoveNumber = fullmove;
	}
	else
		copy.fullmoveNumber = 1;

	*this = copy;
	return true;
}

void PositionSetupDialog::PositionState::setSquare(int row, int column, QChar piece)
{
	if (!isValidPieceSymbol(piece))
		piece = QLatin1Char('.');
	squares[row][column] = piece;
}

QChar PositionSetupDialog::PositionState::squareAt(int row, int column) const
{
	return squares[row][column];
}

PositionSetupDialog::PositionSetupDialog(QWidget* parent)
	: QDialog(parent),
	  ui(new Ui::PositionSetupDialog),
	  m_lastSquare(qMakePair(-1, -1))
{
	ui->setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	initializeBoardTable();

	const struct { const char* label; QChar symbol; } pieces[] = {
		{ QT_TR_NOOP("Empty square"), QLatin1Char('.') },
		{ QT_TR_NOOP("White king (K)"), QLatin1Char('K') },
		{ QT_TR_NOOP("White queen (Q)"), QLatin1Char('Q') },
		{ QT_TR_NOOP("White rook (R)"), QLatin1Char('R') },
		{ QT_TR_NOOP("White bishop (B)"), QLatin1Char('B') },
		{ QT_TR_NOOP("White knight (N)"), QLatin1Char('N') },
		{ QT_TR_NOOP("White pawn (P)"), QLatin1Char('P') },
		{ QT_TR_NOOP("Black king (k)"), QLatin1Char('k') },
		{ QT_TR_NOOP("Black queen (q)"), QLatin1Char('q') },
		{ QT_TR_NOOP("Black rook (r)"), QLatin1Char('r') },
		{ QT_TR_NOOP("Black bishop (b)"), QLatin1Char('b') },
		{ QT_TR_NOOP("Black knight (n)"), QLatin1Char('n') },
		{ QT_TR_NOOP("Black pawn (p)"), QLatin1Char('p') },
	};

	for (const auto& piece : pieces)
		ui->m_pieceSelector->addItem(tr(piece.label), piece.symbol);

	connect(ui->m_boardTable, &QTableWidget::cellClicked,
		this, &PositionSetupDialog::onSquareClicked);
	connect(ui->m_pieceSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &PositionSetupDialog::onPieceSelectionChanged);
	connect(ui->m_clearSquareBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onClearSquare);
	connect(ui->m_resetBoardBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onResetBoard);
	connect(ui->m_clearBoardBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onClearBoard);
	connect(ui->m_loadImageBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onLoadImage);
	connect(ui->m_clearImageBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onClearImage);
	connect(ui->m_copyFenBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onCopyFen);
	connect(ui->m_pasteFenBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onPasteFen);
	connect(ui->m_applyFenBtn, &QPushButton::clicked,
		this, &PositionSetupDialog::onApplyFen);
	connect(ui->m_whiteToMoveRadio, &QRadioButton::toggled,
		this, &PositionSetupDialog::onSideToMoveChanged);
	connect(ui->m_blackToMoveRadio, &QRadioButton::toggled,
		this, &PositionSetupDialog::onSideToMoveChanged);

	const auto castlingBoxes = {
		ui->m_castleK, ui->m_castleQ, ui->m_castlek, ui->m_castleq
	};
	for (auto box : castlingBoxes)
		connect(box, &QCheckBox::toggled,
			this, &PositionSetupDialog::onCastlingChanged);

	connect(ui->m_enPassantEdit, &QLineEdit::textEdited,
		this, &PositionSetupDialog::onEnPassantEdited);
	connect(ui->m_halfMoveSpin, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &PositionSetupDialog::onHalfmoveChanged);
	connect(ui->m_fullMoveSpin, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &PositionSetupDialog::onFullmoveChanged);

	applyFenString(kDefaultFen);
}

PositionSetupDialog::~PositionSetupDialog()
{
	delete ui;
}

void PositionSetupDialog::setFen(const QString& fen)
{
	if (fen.trimmed().isEmpty() || !applyFenString(fen))
		applyFenString(kDefaultFen);
}

QString PositionSetupDialog::fen() const
{
	return m_state.toFen();
}

void PositionSetupDialog::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);
	refreshReferencePreview();
}

void PositionSetupDialog::initializeBoardTable()
{
	ui->m_boardTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->m_boardTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	const QColor lightSquare(0xf0, 0xd9, 0xb5);
	const QColor darkSquare(0xb5, 0x88, 0x63);

	for (int row = 0; row < 8; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			auto* item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsEnabled);
			item->setBackground(((row + col) % 2 == 0) ? lightSquare : darkSquare);
			item->setForeground(Qt::black);
			ui->m_boardTable->setItem(row, col, item);
		}
	}
}

void PositionSetupDialog::refreshBoardCells()
{
	for (int row = 0; row < 8; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			auto* item = ui->m_boardTable->item(row, col);
			if (item == nullptr)
				continue;
			const QChar symbol = m_state.squareAt(row, col);
			item->setText(symbol == QLatin1Char('.') ? QString() : QString(symbol));
		}
	}
}

void PositionSetupDialog::updateFenField()
{
	QSignalBlocker blocker(ui->m_fenLineEdit);
	ui->m_fenLineEdit->setText(m_state.toFen());
}

bool PositionSetupDialog::applyFenString(const QString& fen)
{
	PositionState candidate;
	if (!candidate.fromFen(fen))
	{
		QMessageBox::warning(this, tr("Invalid FEN"),
				     tr("The provided FEN string is not valid:\n%1").arg(fen));
		return false;
	}

	m_state = candidate;
	updateControlsFromState();
	refreshBoardCells();
	updateFenField();
	return true;
}

void PositionSetupDialog::updateControlsFromState()
{
	{
		QSignalBlocker blocker(ui->m_whiteToMoveRadio);
		ui->m_whiteToMoveRadio->setChecked(m_state.whiteToMove);
	}
	{
		QSignalBlocker blocker(ui->m_blackToMoveRadio);
		ui->m_blackToMoveRadio->setChecked(!m_state.whiteToMove);
	}

	const auto setBox = [](QCheckBox* box, bool checked)
	{
		QSignalBlocker blocker(box);
		box->setChecked(checked);
	};
	setBox(ui->m_castleK, m_state.castleK);
	setBox(ui->m_castleQ, m_state.castleQ);
	setBox(ui->m_castlek, m_state.castlek);
	setBox(ui->m_castleq, m_state.castleq);

	{
		QSignalBlocker blocker(ui->m_enPassantEdit);
		ui->m_enPassantEdit->setText(m_state.enPassant == QLatin1String("-")
			? QString() : m_state.enPassant);
	}

	{
		QSignalBlocker blocker(ui->m_halfMoveSpin);
		ui->m_halfMoveSpin->setValue(m_state.halfmoveClock);
	}
	{
		QSignalBlocker blocker(ui->m_fullMoveSpin);
		ui->m_fullMoveSpin->setValue(m_state.fullmoveNumber);
	}

	refreshBoardCells();
	updateFenField();
}

void PositionSetupDialog::syncStateFromControls()
{
	m_state.whiteToMove = ui->m_whiteToMoveRadio->isChecked();
	m_state.castleK = ui->m_castleK->isChecked();
	m_state.castleQ = ui->m_castleQ->isChecked();
	m_state.castlek = ui->m_castlek->isChecked();
	m_state.castleq = ui->m_castleq->isChecked();
	m_state.enPassant = normalizedEnPassant(ui->m_enPassantEdit->text());
	if (m_state.enPassant == QLatin1String("-"))
		ui->m_enPassantEdit->clear();
	else
	{
		QSignalBlocker blocker(ui->m_enPassantEdit);
		ui->m_enPassantEdit->setText(m_state.enPassant);
	}
	m_state.halfmoveClock = ui->m_halfMoveSpin->value();
	m_state.fullmoveNumber = ui->m_fullMoveSpin->value();
	updateFenField();
}

QChar PositionSetupDialog::selectedPieceSymbol() const
{
	QVariant data = ui->m_pieceSelector->currentData();
	if (data.canConvert<QChar>())
		return data.toChar();
	return QLatin1Char('.');
}

QString PositionSetupDialog::normalizedEnPassant(const QString& text) const
{
	QString trimmed = text.trimmed().toLower();
	if (trimmed.isEmpty())
		return QStringLiteral("-");
	if (trimmed == QLatin1String("-"))
		return QStringLiteral("-");
	return isValidEnPassantSquare(trimmed) ? trimmed : QStringLiteral("-");
}

void PositionSetupDialog::setReferenceImage(const QString& path)
{
	if (path.isEmpty())
		return;

	QPixmap pixmap(path);
	if (pixmap.isNull())
	{
		QMessageBox::warning(this, tr("Image error"),
				     tr("Unable to load image: %1").arg(path));
		return;
	}

	m_referenceImagePath = path;
	m_referencePixmap = pixmap;
	ui->m_referenceImageLabel->setText(QString());
	refreshReferencePreview();
}

void PositionSetupDialog::refreshReferencePreview()
{
	if (m_referencePixmap.isNull())
		return;

	const QSize target = ui->m_referenceImageLabel->size();
	if (target.isEmpty())
		return;

	ui->m_referenceImageLabel->setPixmap(
		m_referencePixmap.scaled(target, Qt::KeepAspectRatio,
					 Qt::SmoothTransformation));
}

void PositionSetupDialog::onSquareClicked(int row, int column)
{
	const QChar symbol = selectedPieceSymbol();
	m_state.setSquare(row, column, symbol);
	m_lastSquare = qMakePair(row, column);
	refreshBoardCells();
	updateFenField();
}

void PositionSetupDialog::onPieceSelectionChanged(int index)
{
	Q_UNUSED(index);
}

void PositionSetupDialog::onClearSquare()
{
	if (m_lastSquare.first < 0 || m_lastSquare.second < 0)
		return;
	m_state.setSquare(m_lastSquare.first, m_lastSquare.second, QLatin1Char('.'));
	refreshBoardCells();
	updateFenField();
}

void PositionSetupDialog::onResetBoard()
{
	applyFenString(kDefaultFen);
}

void PositionSetupDialog::onClearBoard()
{
	m_state.clearBoard();
	m_state.whiteToMove = true;
	m_state.castleK = m_state.castleQ = m_state.castlek = m_state.castleq = false;
	m_state.enPassant = QStringLiteral("-");
	m_state.halfmoveClock = 0;
	m_state.fullmoveNumber = 1;
	refreshBoardCells();
	updateControlsFromState();
}

void PositionSetupDialog::onLoadImage()
{
	const QString file = QFileDialog::getOpenFileName(this,
		tr("Select reference image"),
		QString(),
		tr("Image files (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;All files (*)"));
	if (!file.isEmpty())
		setReferenceImage(file);
}

void PositionSetupDialog::onClearImage()
{
	m_referenceImagePath.clear();
	m_referencePixmap = QPixmap();
	ui->m_referenceImageLabel->setPixmap(QPixmap());
	ui->m_referenceImageLabel->setText(tr("No image loaded"));
}

void PositionSetupDialog::onCopyFen()
{
	QGuiApplication::clipboard()->setText(m_state.toFen());
}

void PositionSetupDialog::onPasteFen()
{
	const QString text = QGuiApplication::clipboard()->text();
	if (!text.isEmpty())
		applyFenString(text);
}

void PositionSetupDialog::onApplyFen()
{
	const QString text = ui->m_fenLineEdit->text();
	if (!text.isEmpty())
		applyFenString(text);
}

void PositionSetupDialog::onSideToMoveChanged()
{
	syncStateFromControls();
}

void PositionSetupDialog::onCastlingChanged()
{
	syncStateFromControls();
}

void PositionSetupDialog::onEnPassantEdited(const QString& text)
{
	Q_UNUSED(text);
	syncStateFromControls();
}

void PositionSetupDialog::onHalfmoveChanged(int value)
{
	Q_UNUSED(value);
	syncStateFromControls();
}

void PositionSetupDialog::onFullmoveChanged(int value)
{
	Q_UNUSED(value);
	syncStateFromControls();
}

