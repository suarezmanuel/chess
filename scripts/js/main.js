import { isValidMove } from "./validMoves.js"

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

    const boardSize = 400; // Match the board size from the CSS
    const squareSize = boardSize / 8; // Calculate the size of each square (50px in this case)

    document.querySelectorAll('div[class*="square-"]').forEach(square => {
        const className = Array.from(square.classList).find(cls => cls.startsWith("square-"));
        const [, x, y] = className.match(/square-(\d)(\d)/);

        square.style.transform = `translate(${(x - 1) * squareSize}px, ${(8 - y) * squareSize}px)`; // Adjust to board size
    });

    let draggedPiece = null;

    // Handle drag start
    document.querySelectorAll('.piece').forEach(piece => {
        piece.addEventListener('dragstart', (e) => {
            draggedPiece = e.target; // Save the piece being dragged
            setTimeout(() => {
                draggedPiece.style.opacity = "0.5";
            }, 0);
        });

        piece.addEventListener('dragend', () => {
            draggedPiece.style.opacity = "1";
            draggedPiece = null;
        });
    });

    // Handle the drop event
    document.querySelector('.board').addEventListener('drop', (e) => {
        e.preventDefault();
        if (draggedPiece) {
            const rect = e.target.getBoundingClientRect();
            const squareSize = rect.width / 8;

            const currentClass = Array.from(draggedPiece.classList).find(cls => cls.startsWith("square-"));
            let [, startX, startY] = currentClass.match(/square-(\d)(\d)/);
            startX = parseInt(startX)-1;
            startY = 8-parseInt(startY);
            const targetX = Math.floor((e.clientX - rect.left) / squareSize);
            const targetY = Math.floor((e.clientY - rect.top) / squareSize);

            const pieceType = boardArray[startY][startX];

            // Validate the move
            if (isValidMove(boardArray, pieceType, startX, startY, targetX, targetY)) {
                // Capture if there's a piece at the destination
                if (boardArray[targetY][targetX] !== null) {
                    document.querySelector(`.square-${targetX+1}${8-targetY}`).remove();
                }

                // Move the piece on the board
                draggedPiece.style.transform = translate(`${targetX * squareSize}px`, `${targetY * squareSize}px`);
                
                // Update the board array
                boardArray[targetY][targetX] = boardArray[startY][startX];
                boardArray[startY][startX] = null;

                // Update the class of the dragged piece
                draggedPiece.classList.remove(currentClass);
                draggedPiece.classList.add(`square-${targetX+1}${8-targetY}`);
            } else {
                console.log("Invalid move!");
            }
        }
    });

    document.querySelector('.board').addEventListener('dragover', (e) => {
        e.preventDefault();
    });
});