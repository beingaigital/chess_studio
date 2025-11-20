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

#ifndef POSITIONSETUPDLG_H
#define POSITIONSETUPDLG_H

#include <QDialog>
#include <QPair>
#include <QPixmap>

namespace Ui {
	class PositionSetupDialog;
}
class QResizeEvent;

class PositionSetupDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit PositionSetupDialog(QWidget* parent = nullptr);
		virtual ~PositionSetupDialog();

		void setFen(const QString& fen);
		QString fen() const;

	protected:
		void resizeEvent(QResizeEvent* event) override;

	private slots:
		void onSquareClicked(int row, int column);
		void onPieceSelectionChanged(int index);
		void onClearSquare();
		void onResetBoard();
		void onClearBoard();
		void onLoadImage();
		void onClearImage();
		void onCopyFen();
		void onPasteFen();
		void onApplyFen();
		void onSideToMoveChanged();
		void onCastlingChanged();
		void onEnPassantEdited(const QString& text);
		void onHalfmoveChanged(int value);
		void onFullmoveChanged(int value);

	private:
		struct PositionState
		{
			PositionState();

			void reset();
			void clearBoard();
			QString toFen() const;
			bool fromFen(const QString& fen);
			void setSquare(int row, int column, QChar piece);
			QChar squareAt(int row, int column) const;

			QChar squares[8][8];
			bool whiteToMove;
			bool castleK;
			bool castleQ;
			bool castlek;
			bool castleq;
			QString enPassant;
			int halfmoveClock;
			int fullmoveNumber;
		};

		void initializeBoardTable();
		void refreshBoardCells();
		void updateFenField();
		bool applyFenString(const QString& fen);
		void updateControlsFromState();
		void syncStateFromControls();
		QChar selectedPieceSymbol() const;
		QString normalizedEnPassant(const QString& text) const;
		void setReferenceImage(const QString& path);
		void refreshReferencePreview();

		Ui::PositionSetupDialog* ui;
		PositionState m_state;
		QPair<int, int> m_lastSquare;
		QString m_referenceImagePath;
		QPixmap m_referencePixmap;
};

#endif // POSITIONSETUPDLG_H

