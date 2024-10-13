// main2.js

import { io } from 'https://cdn.socket.io/4.5.4/socket.io.esm.min.js';
import { connectBot } from './ai.js';

let possibleMoves = [];
let startX;
let startY;
let pieceStr;
let playerColor;
let previousGameState = null;
let gameEnded = false;
let currentTurn = 'w';
let draggedPiece = null;

const socket = io();

// Extract the room code from the URL
const urlParams = new URLSearchParams(window.location.search);
const roomCode = urlParams.get('room');

// Join the room
if (roomCode) {
    socket.emit('joinRoom', roomCode);
} else {
    socket.emit('joinRoom', 807);
    connectBot();
}

document.addEventListener("DOMContentLoaded", () => {
    const boardElement = document.querySelector('.board');
    const boardSize = 400; // Match the board size from the CSS
    const squareSize = boardSize / 8; // Each square is 50px

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
                    if (playerColor && color === playerColor && color === currentTurn) {
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

    // Function to handle drag start
    function dragStartHandler(e) {
        // For macOS to not show the plus sign
        e.dataTransfer.effectAllowed = 'move';
        const rect = boardElement.getBoundingClientRect();
        const squareSize = rect.width / 8;

        startX = Math.floor((e.clientX - rect.left) / squareSize);
        startY = Math.floor((e.clientY - rect.top) / squareSize);

        draggedPiece = e.target;
        pieceStr = Array.from(draggedPiece.classList).find(cls => cls.length === 2);

        // Check if it's player's piece
        if (pieceStr.charAt(0) !== playerColor || !pieceStr) {
            // Not this player's piece, prevent dragging
            e.preventDefault();
            return;
        }

        setTimeout(() => {
            draggedPiece.style.opacity = "0";
        }, 0);

        let movesToShow = [];
        possibleMoves.forEach(move => {
            if (move[0] === startX && move[1] === startY) movesToShow.push(move);
        });
        showHints(movesToShow);
    }

    // Function to handle drag end
    function dragEndHandler() {
        if (draggedPiece) {
            draggedPiece.style.opacity = "1";
            draggedPiece = null;
        }
    }

    // Attach event listeners to pieces
    function attachEventListeners() {
        document.querySelectorAll('.piece').forEach(piece => {
            piece.addEventListener('dragstart', dragStartHandler);
            piece.addEventListener('dragend', dragEndHandler);
        });
    }

    attachEventListeners();

    // Function to show hints for possible moves
    function showHints(moves) {
        clearHints(); // Clear any existing hints

        moves.forEach(move => {
            const hint = document.createElement('div');

            // let x = playerColor === 'w' ? move[2] : 7-move[2];
            // let y = playerColor === 'w' ? move[3] : 7-move[3];
            let x = move[2];
            let y = move[3];
            const targetSquareClass = `square-${x + 1}${8 - y}`;
            const pieceAtTarget = document.querySelector(`.piece.${targetSquareClass}`);

            if (pieceAtTarget) {
                hint.classList.add('capture-hint');
            } else {
                hint.classList.add('hint');
            }

            // Get the square position for the move
            const { left, top } = getSquarePosition(x, y);

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

            // Check if the move is valid
            const isValidMove = possibleMoves.some(move =>
                move[0] === startX && move[1] === startY && move[2] === targetX && move[3] === targetY
            );

            if (isValidMove) {
                // Save game state before making the move
                saveGameState();

                // Optimistically update the UI
                movePieceUI(currentClass, startX, startY, targetX, targetY);

                // Send the move to the server
                let sx = playerColor === 'w' ? startX : 7-startX;
                let sy = playerColor === 'w' ? startY : 7-startY;
                let tx = playerColor === 'w' ? targetX : 7-targetX;
                let ty = playerColor === 'w' ? targetY : 7-targetY;

                socket.emit('moveMade', { startX: sx, startY: sy, targetX: tx, targetY: ty, playerColor });
            } else {
                console.log("Invalid move!");
            }

            updateDraggablePieces();
        }
    });

    // Function to save game state (in case move is invalidated)
    function saveGameState() {
        previousGameState = boardElement.innerHTML;
    }

    // Function to restore game state
    function restoreGameState() {
        if (previousGameState) {
            boardElement.innerHTML = previousGameState;
            previousGameState = null;
            attachEventListeners();
        }
    }

    function movePieceUI(currentClass, startX, startY, targetX, targetY) {
        let draggedPiece = document.getElementsByClassName(currentClass)[0];
        let pieceClasses = Array.from(draggedPiece.classList);
        let pieceType = pieceClasses.find(cls => cls.length === 2);
        let color = pieceType.charAt(0);
        let piece = pieceType.charAt(1);

        // Capture if there's a piece at the destination
        const toSquareClass = `square-${targetX + 1}${8 - targetY}`;
        const capturedPiece = document.querySelector(`.piece.${toSquareClass}`);
        if (capturedPiece && capturedPiece !== draggedPiece) capturedPiece.remove();

        // Update the class of the dragged piece
        draggedPiece.classList.remove(currentClass);
        draggedPiece.classList.add(toSquareClass);

        // Update the piece's position
        draggedPiece.style.left = `${targetX * squareSize}px`;
        draggedPiece.style.top = `${targetY * squareSize}px`;

        // Pawn promotion
        if (piece === 'p' && (targetY === 0 || targetY === 7)) {
            piece = 'q';
            pieceType = color + piece;
            draggedPiece.classList.remove('wp', 'bp');
            draggedPiece.classList.add(pieceType);
        }

        // En passant
        if (piece === 'p') {
            let y = targetY + 1;
            if (Math.abs(targetX - startX) === 1) {
                if (!capturedPiece) {
                    let capturedPawnClass = `square-${targetX + 1}${8 - y}`;
                    let capturedPawn = document.querySelector(`.piece.${capturedPawnClass}`);
                    if (capturedPawn) capturedPawn.remove();
                }
            }
        }

        // Check for castle
        if (piece === 'k' && Math.abs(targetX - startX) > 1) {
            let tempTargetX = playerColor === 'w' ? targetX : 7-targetX;
            let rookStartX, rookStartY, rookTargetX, rookTargetY;
            if (color === 'w') {
                if (tempTargetX === 6) {
                    rookStartX = 7;
                    rookStartY = 7;
                    rookTargetX = 5;
                    rookTargetY = 7;
                } else {
                    rookStartX = 0;
                    rookStartY = 7;
                    rookTargetX = 3;
                    rookTargetY = 7;
                }
            } else {
                if (tempTargetX === 6) {
                    rookStartX = 7;
                    rookStartY = 0;
                    rookTargetX = 5;
                    rookTargetY = 0;
                } else {
                    rookStartX = 0;
                    rookStartY = 0;
                    rookTargetX = 3;
                    rookTargetY = 0;
                }
            }
            
            rookStartX = playerColor === 'w' ? rookStartX : 7-rookStartX;
            rookStartY = playerColor === 'w' ? rookStartY : 7-rookStartY;
            rookTargetX = playerColor === 'w' ? rookTargetX : 7-rookTargetX;
            rookTargetY = playerColor === 'w' ? rookTargetY : 7-rookTargetY;

            let rookStartClass = `square-${rookStartX + 1}${8 - rookStartY}`;
            let rookTargetClass = `square-${rookTargetX + 1}${8 - rookTargetY}`;
            let rook = document.querySelector(`.piece.${rookStartClass}`);
            rook.classList.replace(rookStartClass, rookTargetClass);
            rook.style.left = `${rookTargetX * squareSize}px`;
            rook.style.top = `${rookTargetY * squareSize}px`;
        }

        handleHighlights(boardElement, startX, startY, targetX, targetY);
    }

    socket.on('moveValidation', (e) => {
        if (e.response === 'invalid move') {
            restoreGameState();
            alert(`Invalid move: ${e.reason}`);
        }
        // No action needed for valid move since we've updated the UI
    });

    socket.on('playerColor', (data) => {
        playerColor = data.color;
        console.log('Assigned color:', playerColor);
        updateDraggablePieces();
    });

    function createPieceDivs(positions, pieceColor) {
        positions.forEach((piece, coordinates) => {
            let [x, y] = coordinates; // Extract the x and y coordinates
            const div = document.createElement('div');
            x = playerColor === 'w' ? x : 7-x;
            y = playerColor === 'w' ? y : 7-y;
            div.className = `piece ${pieceColor}${piece} square-${x+1}${8-y}`;
            div.setAttribute('draggable', 'true');
            div.style = ''; // Add any inline styling if needed
    
            // Assuming you want to append these divs to an existing container, e.g., a board element
            document.querySelector('.board').appendChild(div);
        });
    }

    socket.on('positions', (data) => {
        // showing the pieces as they appear in the server
        document.querySelectorAll(".piece").forEach(div => div.remove());
        document.querySelectorAll(".highlight").forEach(div => div.remove())

        createPieceDivs(new Map(data.white), 'w');
        createPieceDivs(new Map(data.black), 'b');

        document.querySelectorAll('.piece').forEach(piece => {
            const className = Array.from(piece.classList).find(cls => cls.startsWith("square-"));
            const [, x, y] = className.match(/square-(\d)(\d)/);
            const posX = (parseInt(x) - 1) * squareSize;
            const posY = (8 - parseInt(y)) * squareSize;
    
            piece.style.left = `${posX}px`;
            piece.style.top = `${posY}px`;
        });

        updateDraggablePieces();
        attachEventListeners();
    });

    // If opponent made a move
    socket.on('moveMade', (e) => {
        if (e.playerColor !== playerColor) {
            let startX = playerColor === 'w' ? e.startX : 7-e.startX;
            let startY = playerColor === 'w' ? e.startY : 7-e.startY;
            let targetX = playerColor === 'w' ? e.targetX : 7-e.targetX;
            let targetY = playerColor === 'w' ? e.targetY: 7-e.targetY;

            const fromSquareClass = `square-${startX + 1}${8 - startY}`;
            movePieceUI(fromSquareClass, startX, startY, targetX, targetY);
            // Update current turn
            currentTurn = (currentTurn === 'w') ? 'b' : 'w';
            updateDraggablePieces();
        }
    });

    socket.on('gameEnded', (e) => {
        gameEnded = true;
        alert(`${e.message}! ${e.winner === 'none' ? 'It\'s a draw.' : `${e.winner} wins!`}`);
        updateDraggablePieces();
    });

    socket.on('receivePossibleMoves', (e) => {
        possibleMoves = e.possibleMoves;
        if (playerColor === 'b') {
            let moves = [];
            possibleMoves.forEach(move => {
                moves.push([7-move[0], 7-move[1], 7-move[2], 7-move[3]]);
            });
            possibleMoves = moves;
        }
        // Update current turn
        currentTurn = playerColor;
        updateDraggablePieces();
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
});
