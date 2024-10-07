import { isValidMove, isValidCastling } from "./validMoves.js"

const boardArray = [
    ['br', 'bn', 'bb', 'bq', 'bk', 'bb', 'bn', 'br'],
    ['bp', 'bp', 'bp', 'bp', 'bp', 'bp', 'bp', 'bp'],
    [null, null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null, null],
    ['wp', 'wp', 'wp', 'wp', 'wp', 'wp', 'wp', 'wp'],
    ['wr', 'wn', 'wb', 'wq', 'wk', 'wb', 'wn', 'wr']
];

document.addEventListener("DOMContentLoaded", () => {
    const boardElement = document.querySelector('.board');
    const boardSize = 400; // Match the board size from the CSS
    const squareSize = boardSize / 8; // Each square is 50px
    let currentTurn = 'w';
    let whiteCastleLeft = true;
    let whiteCastleRight = true;
    let blackCastleLeft = true;
    let blackCastleRight = true;

    // Position the pieces on the board
    document.querySelectorAll('.piece').forEach(piece => {
        const className = Array.from(piece.classList).find(cls => cls.startsWith("square-"));
        const [, x, y] = className.match(/square-(\d)(\d)/);
        const posX = (parseInt(x) - 1) * squareSize;
        const posY = (8 - parseInt(y)) * squareSize;

        piece.style.left = `${posX}px`;
        piece.style.top = `${posY}px`;
    });

    let draggedPiece = null;

    // Function to update the draggability of pieces
    function updateDraggablePieces() {
        document.querySelectorAll('.piece').forEach(piece => {
            const pieceType = Array.from(piece.classList).find(cls => 
                ['wp', 'wr', 'wn', 'wb', 'wq', 'wk', 'bp', 'br', 'bn', 'bb', 'bq', 'bk'].includes(cls)
            );
            if (pieceType) {
                const color = pieceType.charAt(0); // 'w' or 'b'
                if (color === currentTurn) {
                    piece.setAttribute('draggable', true);
                    piece.classList.add('draggable');
                } else {
                    piece.setAttribute('draggable', false);
                    piece.classList.remove('draggable');
                }
            }
        });
    }

    // Call the function to set initial draggability
    updateDraggablePieces();

    // Handle drag start
    document.querySelectorAll('.piece').forEach(piece => {
        piece.addEventListener('dragstart', (e) => {
            // For macOS to not show the plus sign
            e.dataTransfer.effectAllowed = 'move';

            const pieceType = Array.from(e.target.classList).find(cls => 
                ['wp', 'wr', 'wn', 'wb', 'wq', 'wk', 'bp', 'br', 'bn', 'bb', 'bq', 'bk'].includes(cls)
            );
            if (pieceType) {
                const color = pieceType.charAt(0); // 'w' or 'b'
                if (color !== currentTurn) {
                    // Not this player's turn, prevent dragging
                    e.preventDefault();
                    return;
                }
            }

            draggedPiece = e.target; // Save the piece being dragged
            setTimeout(() => {
                draggedPiece.style.opacity = "0.5";
            }, 0);
        });

        piece.addEventListener('dragend', () => {
            if (draggedPiece) {
                draggedPiece.style.opacity = "1";
                draggedPiece = null;
            }
        });
    });

    // Function to get square position (left, top) based on x and y indices
    function getSquarePosition(x, y) {
        return {
            left: x * squareSize,
            top: y * squareSize
        };
    }

    // Handle the drop event
    boardElement.addEventListener('drop', (e) => {
        e.preventDefault();
        e.dataTransfer.dropEffect = 'move';

        if (draggedPiece) {
            const rect = boardElement.getBoundingClientRect();
            const squareSize = rect.width / 8;

            const currentClass = Array.from(draggedPiece.classList).find(cls => cls.startsWith("square-"));
            let [, startX, startY] = currentClass.match(/square-(\d)(\d)/);
            startX = parseInt(startX) - 1;
            startY = 8 - parseInt(startY);

            const targetX = Math.floor((e.clientX - rect.left) / squareSize);
            const targetY = Math.floor((e.clientY - rect.top) / squareSize);

            const pieceType = boardArray[startY][startX];
            // remove ability to castle if eaten
            // **Handle Castling**
            if (pieceType !== null && pieceType[1] === 'k'
                && (whiteCastleLeft || whiteCastleRight || blackCastleLeft || blackCastleRight)
                && isValidCastling(boardArray, pieceType[0], startX, startY, targetX, targetY,
                                   whiteCastleLeft, whiteCastleRight, blackCastleLeft, blackCastleRight)) {

                // Update castling rights
                if (pieceType[0] === 'w') {
                    whiteCastleLeft = whiteCastleRight = false;
                } else {
                    blackCastleLeft = blackCastleRight = false;
                }

                // Determine if it's kingside or queenside castling
                const isKingsideCastling = (targetX - startX === 2);
                const isQueensideCastling = (targetX - startX === -2);

                let rookStartX, rookStartY, rookTargetX, rookTargetY;

                // The rook is on the same rank (row) as the king
                rookStartY = startY;
                rookTargetY = startY;

                if (isKingsideCastling) {
                    // Kingside castling
                    rookStartX = 7; // h-file
                    rookTargetX = startX + 1; // Rook moves next to the king
                } else if (isQueensideCastling) {
                    // Queenside castling
                    rookStartX = 0; // a-file
                    rookTargetX = startX - 1; // Rook moves next to the king
                } else {
                    console.log("Invalid castling move!");
                    return;
                }

                // Update the board array for king and rook
                boardArray[targetY][targetX] = boardArray[startY][startX]; // Move king
                boardArray[startY][startX] = null; // Empty the king's starting square

                boardArray[rookTargetY][rookTargetX] = boardArray[rookStartY][rookStartX]; // Move rook
                boardArray[rookStartY][rookStartX] = null; // Empty the rook's starting square

                // Move the king piece in the DOM
                draggedPiece.style.left = `${targetX * squareSize}px`;
                draggedPiece.style.top = `${targetY * squareSize}px`;

                // Update the class of the king piece
                draggedPiece.classList.remove(currentClass);
                draggedPiece.classList.add(`square-${targetX + 1}${8 - targetY}`);

                // Move the rook piece in the DOM
                const rookClass = `.piece.square-${rookStartX + 1}${8 - rookStartY}`;
                const rookPiece = document.querySelector(rookClass);
                if (rookPiece) {
                    rookPiece.style.left = `${rookTargetX * squareSize}px`;
                    rookPiece.style.top = `${rookTargetY * squareSize}px`;

                    // Update the class of the rook piece
                    rookPiece.classList.remove(`square-${rookStartX + 1}${8 - rookStartY}`);
                    rookPiece.classList.add(`square-${rookTargetX + 1}${8 - rookTargetY}`);
                } else {
                    console.log("Rook piece not found for castling!");
                }

                // Handle the highlights
                // Remove existing highlights first
                document.querySelectorAll('.highlight').forEach(el => el.remove());

                // Get positions for the highlights
                const kingStartPos = getSquarePosition(startX, startY);
                const kingTargetPos = getSquarePosition(targetX, targetY);
                const rookStartPos = getSquarePosition(rookStartX, rookStartY);
                const rookTargetPos = getSquarePosition(rookTargetX, rookTargetY);

                // Create and position the highlight for the king's start square
                const kingStartHighlight = document.createElement('div');
                kingStartHighlight.classList.add('highlight');
                kingStartHighlight.style.left = `${kingStartPos.left}px`;
                kingStartHighlight.style.top = `${kingStartPos.top}px`;
                kingStartHighlight.style.width = `${squareSize}px`;
                kingStartHighlight.style.height = `${squareSize}px`;

                // Create and position the highlight for the king's target square
                const kingTargetHighlight = document.createElement('div');
                kingTargetHighlight.classList.add('highlight');
                kingTargetHighlight.style.left = `${kingTargetPos.left}px`;
                kingTargetHighlight.style.top = `${kingTargetPos.top}px`;
                kingTargetHighlight.style.width = `${squareSize}px`;
                kingTargetHighlight.style.height = `${squareSize}px`;

                // Create and position the highlight for the rook's start square
                const rookStartHighlight = document.createElement('div');
                rookStartHighlight.classList.add('highlight');
                rookStartHighlight.style.left = `${rookStartPos.left}px`;
                rookStartHighlight.style.top = `${rookStartPos.top}px`;
                rookStartHighlight.style.width = `${squareSize}px`;
                rookStartHighlight.style.height = `${squareSize}px`;

                // Create and position the highlight for the rook's target square
                const rookTargetHighlight = document.createElement('div');
                rookTargetHighlight.classList.add('highlight');
                rookTargetHighlight.style.left = `${rookTargetPos.left}px`;
                rookTargetHighlight.style.top = `${rookTargetPos.top}px`;
                rookTargetHighlight.style.width = `${squareSize}px`;
                rookTargetHighlight.style.height = `${squareSize}px`;

                // Append the highlights to the board
                boardElement.appendChild(kingStartHighlight);
                boardElement.appendChild(kingTargetHighlight);
                boardElement.appendChild(rookStartHighlight);
                boardElement.appendChild(rookTargetHighlight);

                // Switch turn to the other player
                currentTurn = (currentTurn === 'w') ? 'b' : 'w';
                updateDraggablePieces();

                return; // Exit the drop event handler
            }

            // **Handle Normal Moves**
            // Validate the move
            if (isValidMove(boardArray, pieceType, startX, startY, targetX, targetY)) {
                // Update castling rights if the king or rook moves
                if (pieceType != null) {
                    // If king moves
                    if (pieceType[1] === 'k') {
                        if (pieceType[0] === 'w')
                            whiteCastleLeft = whiteCastleRight = false;
                        else
                            blackCastleLeft = blackCastleRight = false;
                    // If rook moves
                    } else if (pieceType[1] === 'r') {
                        if (pieceType[0] === 'w') {
                            if (startX == 0) whiteCastleLeft = false;
                            if (startX == 7) whiteCastleRight = false;
                        } else {
                            if (startX == 0) blackCastleLeft = false;
                            if (startX == 7) blackCastleRight = false;
                        }
                    }
                }

                // Capture if there's a piece at the destination
                if (boardArray[targetY][targetX] !== null) {
                    const capturedPiece = document.querySelector(`.square-${targetX + 1}${8 - targetY}.piece`);
                    if (capturedPiece) capturedPiece.remove();
                }

                // Move the piece on the board
                draggedPiece.style.left = `${targetX * squareSize}px`;
                draggedPiece.style.top = `${targetY * squareSize}px`;

                // Update the board array
                boardArray[targetY][targetX] = boardArray[startY][startX];
                boardArray[startY][startX] = null;

                // Update the class of the dragged piece
                draggedPiece.classList.remove(currentClass);
                draggedPiece.classList.add(`square-${targetX + 1}${8 - targetY}`);

                // Handle the highlights
                // Remove existing highlights first
                document.querySelectorAll('.highlight').forEach(el => el.remove());

                // Get positions for the highlights
                const startPos = getSquarePosition(startX, startY);
                const targetPos = getSquarePosition(targetX, targetY);

                // Create and position the highlight for the start square
                const startHighlight = document.createElement('div');
                startHighlight.classList.add('highlight');
                startHighlight.style.left = `${startPos.left}px`;
                startHighlight.style.top = `${startPos.top}px`;
                startHighlight.style.width = `${squareSize}px`;
                startHighlight.style.height = `${squareSize}px`;

                // Create and position the highlight for the target square
                const targetHighlight = document.createElement('div');
                targetHighlight.classList.add('highlight');
                targetHighlight.style.left = `${targetPos.left}px`;
                targetHighlight.style.top = `${targetPos.top}px`;
                targetHighlight.style.width = `${squareSize}px`;
                targetHighlight.style.height = `${squareSize}px`;

                // Append the highlights to the board
                boardElement.appendChild(startHighlight);
                boardElement.appendChild(targetHighlight);

                // Switch turn to the other player
                currentTurn = (currentTurn === 'w') ? 'b' : 'w';
                updateDraggablePieces();
            } else {
                console.log("Invalid move!");
            }
        }
    });

    boardElement.addEventListener('dragover', (e) => {
        e.preventDefault();
        e.dataTransfer.dropEffect = 'move';
    });
});
