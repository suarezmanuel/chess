// Helper function to check if a move is valid
export function isValidMove(boardArray, piece, startX, startY, targetX, targetY) {
    const pieceType = piece.substring(1); // e.g., 'p' for pawn, 'r' for rook, etc.

    switch (pieceType) {
        case 'p': // Pawn move (basic example)
            return isValidPawnMove(boardArray, piece[0], startX, startY, targetX, targetY);
        case 'r': // Rook move
            return isValidRookMove(boardArray, piece[0], startX, startY, targetX, targetY);
        case 'n': // Knight move
            return isValidKnightMove(boardArray, piece[0], startX, startY, targetX, targetY);
        case 'b': // Bishop move
            return isValidBishopMove(boardArray, piece[0], startX, startY, targetX, targetY);
        case 'q': // Queen move
            return isValidQueenMove(boardArray, piece[0], startX, startY, targetX, targetY);
        case 'k': // King move
            return isValidKingMove(boardArray, piece[0], startX, startY, targetX, targetY);
        default:
            return false;
    }
}

// Helper function to check if the path is clear for a rook
function isPathClearForRook(boardArray, startX, startY, targetX, targetY) {
    if (startX === targetX) { // Moving vertically
        const step = targetY > startY ? 1 : -1;
        for (let y = startY + step; y !== targetY; y += step) {
            if (boardArray[y][startX] !== null) {
                return false; // Something is blocking the path
            }
        }
    } else if (startY === targetY) { // Moving horizontally
        const step = targetX > startX ? 1 : -1;
        for (let x = startX + step; x !== targetX; x += step) {
            if (boardArray[startY][x] !== null) {
                return false; // Something is blocking the path
            }
        }
    } else {
        return false; // Invalid rook move
    }
    return true;
}

function isValidPawnMove(boardArray, color, startX, startY, targetX, targetY) {
    
    if (color === 'w') {
        if (targetY === startY - 1 && Math.abs(targetX - startX) === 1) {
            return boardArray[targetY][targetX] !== null;
        }
        if (startX != targetX) return false;
        let moveOnce  = (targetY === startY - 1 && boardArray[startY - 1][targetX] === null);
        let moveTwice = (targetY === startY - 2 && boardArray[startY - 2][targetX] === null);

        if (startY === 6) { return (moveOnce || moveTwice); } 
        return moveOnce;

    } else {
        if (targetY === startY + 1 && Math.abs(targetX - startX) === 1) {
            return boardArray[targetY][targetX] !== null;
        }
        if (startX != targetX) return false;
        let moveOnce  = (targetY === startY + 1 && boardArray[startY + 1][targetX] === null);
        let moveTwice = (targetY === startY + 2 && boardArray[startY + 2][targetX] === null);

        if (startY === 1) { return (moveOnce || moveTwice); } 
        return moveOnce;
    }
}

function isValidRookMove(boardArray, color, startX, startY, targetX, targetY) {
    return (startX === targetX || startY === targetY) && isPathClearForRook(boardArray, startX, startY, targetX, targetY);
}

function isValidKnightMove(boardArray, color, startX, startY, targetX, targetY) {
    const dx = Math.abs(startX - targetX);
    const dy = Math.abs(startY - targetY);
    return ((dx === 2 && dy === 1) || (dx === 1 && dy === 2)) && (boardArray[targetY][targetX] === null || boardArray[targetY][targetX].substring(0) !== color);
}

function isPathClearForBishop(boardArray, startX, startY, targetX, targetY) {
    const deltaX = targetX - startX;
    const deltaY = targetY - startY;

    // Bishops move diagonally, so the absolute changes in x and y must be equal
    if (Math.abs(deltaX) !== Math.abs(deltaY)) {
        return false; // Invalid bishop move
    }

    const stepX = deltaX > 0 ? 1 : -1; // Direction of x movement
    const stepY = deltaY > 0 ? 1 : -1; // Direction of y movement

    // Check each square in the path
    for (let i = 1; i < Math.abs(deltaX); i++) {
        const x = startX + (i * stepX);
        const y = startY + (i * stepY);
        if (boardArray[y][x] !== null) {
            return false; // Something is blocking the path
        }
    }
    return true;
}

function isValidBishopMove(boardArray, piece, startX, startY, targetX, targetY) {

    // Check if the piece is a bishop and the move is diagonal
    if (Math.abs(startX - targetX) === Math.abs(startY - targetY)) {
        return isPathClearForBishop(boardArray, startX, startY, targetX, targetY);
    }
    return false;
}

function isValidQueenMove(boardArray, piece, startX, startY, targetX, targetY) {
    // Check if the move is either vertical, horizontal, or diagonal
    if (startX === targetX || startY === targetY) {
        // Horizontal or vertical move, use rook's path clearing
        return isPathClearForRook(boardArray, startX, startY, targetX, targetY);
    } else if (Math.abs(startX - targetX) === Math.abs(startY - targetY)) {
        // Diagonal move, use bishop's path clearing
        return isPathClearForBishop(boardArray, startX, startY, targetX, targetY);
    }
    return false; // Not a valid queen move
}

function isValidKingMove(boardArray, piece, startX, startY, targetX, targetY) {
    // Calculate the absolute distance from the start position to the target position
    const deltaX = Math.abs(startX - targetX);
    const deltaY = Math.abs(startY - targetY);

    // King can move one square in any direction
    if (deltaX <= 1 && deltaY <= 1) {
        // Add any additional checks if necessary (like checking for check)
        return true; // Valid move
    }
    return false; // Not a valid king move
}
