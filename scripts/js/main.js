import { getValidMoves, checkColor, getValidMovesWhite, getValidMovesBlack, isInCheck } from "./validMoves.js"

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

// {position, piece} 
export const whitePositions = new Map([[[0,6], 'p'], [[1,6], 'p'], [[2,6], 'p'], [[3,6], 'p'], [[4,6], 'p'], [[5,6], 'p'], [[6,6], 'p'], [[7,6], 'p'], 
                                       [[0,7], 'r'], [[1,7], 'n'], [[2,7], 'b'], [[3,7], 'q'], [[4,7], 'k'], [[5,7], 'b'], [[6,7], 'n'], [[7,7], 'r']]);

export const blackPositions = new Map([[[0,1], 'p'], [[1,1], 'p'], [[2,1], 'p'], [[3,1], 'p'], [[4,1], 'p'], [[5,1], 'p'], [[6,1], 'p'], [[7,1], 'p'], 
                                       [[0,0], 'r'], [[1,0], 'n'], [[2,0], 'b'], [[3,0], 'q'], [[4,0], 'k'], [[5,0], 'b'], [[6,0], 'n'], [[7,0], 'r']]);
    
let possibleMoves = [];
let startX;
let startY;
let pieceStr;
let gameEnded = false;
let onCheck = false;
    
export function deleteFromMap(map, arr) {
    map.forEach((value, key) => {
        if (arr[0] == key[0] && arr[1] == key[1]) map.delete(key);
    });
}
    
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
            if (gameEnded) {
                piece.setAttribute('draggable', false);
                piece.classList.remove('draggable');
            } else {
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
            }
        });
    }

    // Call the function to set initial draggability
    updateDraggablePieces();

    boardElement.addEventListener('dragover', (e) => {
        e.preventDefault();
        e.dataTransfer.dropEffect = 'move';
    });

    // Handle drag start
    document.querySelectorAll('.piece').forEach(piece => {
        piece.addEventListener('dragstart', (e) => {
            // For macOS to not show the plus sign
            e.dataTransfer.effectAllowed = 'move';

            const rect = boardElement.getBoundingClientRect();
            const squareSize = rect.width / 8;

            startX = Math.floor((e.clientX - rect.left) / squareSize);
            startY = Math.floor((e.clientY - rect.top) / squareSize);
            pieceStr = boardArray[startY][startX];
            // const pieceType = Array.from(e.target.classList).find(cls => 
            //     ['wp', 'wr', 'wn', 'wb', 'wq', 'wk', 'bp', 'br', 'bn', 'bb', 'bq', 'bk'].includes(cls)
            // );
            if (pieceStr) {
                const color = pieceStr.charAt(0); // 'w' or 'b'
                if (color !== currentTurn) {
                    // Not this player's turn, prevent dragging
                    e.preventDefault();
                    return;
                }
            }

            draggedPiece = e.target;
            setTimeout(() => {
                draggedPiece.style.opacity = "0";
            }, 0);

            // let allMoves = [];  
            // if (pieceStr) {
            //     const color = pieceStr.charAt(0);
            //     if (color == 'w') {
            //         allMoves = getValidMovesWhite(boardArray, pieceStr, startX, startY);
            //         possibleMoves
            //     } else {
            //         allMoves = getValidMovesBlack(boardArray, pieceStr, startX, startY);
            //     }
            //     showHints(allMoves);
            // }

            possibleMoves = getValidMoves(boardArray, pieceStr, startX, startY, onCheck);
            showHints(possibleMoves);
        });


        piece.addEventListener('dragend', () => {
            if (draggedPiece) {
                draggedPiece.style.opacity = "1";
                draggedPiece = null;
            }
        });
    });

    // Function to show hints for possible moves
    function showHints(moves) {
        clearHints(); // Clear any existing hints
        let oppColor = (pieceStr[0] === 'w') ? 'b' : 'w';

        moves.forEach(move => {
            
            const hint = document.createElement('div');
            if (checkColor(boardArray[move[1]][move[0]], oppColor)) {
                hint.classList.add('capture-hint');
            } else {
                hint.classList.add('hint');
            }

            // Get the square position for the move
            const { left, top } = getSquarePosition(move[0], move[1]);

            // Position the hint
            hint.style.left = `${left}px`;
            hint.style.top = `${top}px`;

            // Append the hint to the board
            boardElement.appendChild(hint);
        });
    }

    // Function to clear existing hints
    function clearHints() {
        document.querySelectorAll('.hint').forEach(hint => hint.remove());
        document.querySelectorAll('.capture-hint').forEach(hint => hint.remove());
    }

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
        clearHints();

        if (draggedPiece) {
            const rect = boardElement.getBoundingClientRect();
            const squareSize = rect.width / 8;

            const currentClass = Array.from(draggedPiece.classList).find(cls => cls.startsWith("square-"));
            const targetX = Math.floor((e.clientX - rect.left) / squareSize);
            const targetY = Math.floor((e.clientY - rect.top) / squareSize);

            // move pieces if castling

            if (possibleMoves.some(move => move[0] === targetX && move[1] === targetY)) {
            
                updateCastling();

                // Capture if there's a piece at the destination
                if (boardArray[targetY][targetX] !== null) {
                    const capturedPiece = document.querySelector(`.square-${targetX + 1}${8 - targetY}.piece`);
                    if (capturedPiece) capturedPiece.remove();
                }

                // move the piece, doesn't create a new one
                draggedPiece.style.left = `${targetX * squareSize}px`;
                draggedPiece.style.top  = `${targetY * squareSize}px`;

                // pawn promotion
                if (pieceStr[1] === 'p' && (targetY == 0 || targetY == 7)) { 
                    pieceStr = pieceStr[0] + 'q';
                    draggedPiece.classList.remove('wp', 'bp');
                    draggedPiece.classList.add(pieceStr);
                }
         
                if (pieceStr.charAt(0) == 'w') {
                    deleteFromMap(whitePositions, [startX, startY]);
                    whitePositions.set([targetX, targetY], pieceStr.charAt(1));
                    deleteFromMap(blackPositions, [targetX, targetY]);
                } else {
                    deleteFromMap(blackPositions, [startX, startY]);
                    blackPositions.set([targetX, targetY], pieceStr.charAt(1));
                    deleteFromMap(whitePositions, [targetX, targetY]);
                }

                // Update the board array
                boardArray[targetY][targetX] = pieceStr;
                boardArray[startY][startX] = null;

                let positions = pieceStr.charAt(0) === "b" ? whitePositions : blackPositions;
                let kingPos = [];
                positions.forEach((value, key) => {
                    if (value == 'k') kingPos = key;
                });
                
                // Update the class of the dragged piece
                draggedPiece.classList.remove(currentClass);
                draggedPiece.classList.add(`square-${targetX + 1}${8 - targetY}`);

                handleHighlights(boardElement, startX, startY, targetX, targetY);

                // Switch turn to the other player
                currentTurn = (currentTurn === 'w') ? 'b' : 'w';
                
                onCheck = isInCheck(boardArray, pieceStr.charAt(0) === "w" ? "b" : "w", kingPos);
                // check if we put the oponent in checkmate or stalemate
                let allMoves;
                if (currentTurn === 'w') {
                    allMoves = getValidMovesWhite(boardArray);
                } else {
                    allMoves = getValidMovesBlack(boardArray);
                }
                if (allMoves.length == 0) {
                    if (onCheck) {
                        console.log("checkmate,", ((pieceStr[0] == 'w') ? "white" : "black"), "wins");
                    } else {
                        console.log("stalemate");
                    }
                    gameEnded = true;
                }  
            } else {
                console.log("Invalid move!");
            }

            updateDraggablePieces();
        }
    });

   
    function handleHighlights(boardElement, startX, startY, targetX, targetY) {
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
    }

    function updateCastling() {
        // if rook is taken castling isn't possible
        // if (isValidMove(boardArray, pieceType, startX, startY, targetX, targetY)) {
        // Update castling rights if the king or rook moves
        // if (pieceType != null) {
        //     // If king moves
        //     if (pieceType[1] === 'k') {
        //         if (pieceType[0] === 'w')
        //             whiteCastleLeft = whiteCastleRight = false;
        //         else
        //             blackCastleLeft = blackCastleRight = false;
        //     // If rook moves
        //     } else if (pieceType[1] === 'r') {
        //         if (pieceType[0] === 'w') {
        //             if (startX == 0) whiteCastleLeft = false;
        //             if (startX == 7) whiteCastleRight = false;
        //         } else {
        //             if (startX == 0) blackCastleLeft = false;
        //             if (startX == 7) blackCastleRight = false;
        //         }
        //     }
        // }
    }
});
