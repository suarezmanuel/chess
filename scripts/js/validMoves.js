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

function isValidPawnMove(boardArray, color, startX, startY, targetX, targetY) {
    if (color === 'w') {
        // eat piece not of the same color
        if (targetY === startY - 1 && Math.abs(targetX - startX) === 1) {
            return boardArray[targetY][targetX] != null && boardArray[targetY][targetX][0] != color;
        }
        if (startX != targetX || startY == 0) return false;
        // one forward piece
        let moveOnce  = (targetY === startY - 1 && boardArray[startY - 1][targetX] === null);
        // if didn't move
        if (startY === 6) {
            // two forward piece
            let moveTwice = (targetY === startY - 2 && boardArray[startY - 2][targetX] === null);
            return moveOnce || moveTwice; 
        } 
        return moveOnce;

    } else {
        // eat piece not of the same color
        if (targetY === startY + 1 && Math.abs(targetX - startX) === 1) {
            return boardArray[targetY][targetX] != null && boardArray[targetY][targetX][0] != color;
        }
        if (startX != targetX || startY == 7) return false;
        // one forward piece
        let t1 = boardArray[startY + 1][targetX];
        let moveOnce  = (targetY === startY + 1 && (t1 === null || t1[0] !== color));
        // if didn't move
        if (startY === 1) {
            // two forward piece
            let t2 = boardArray[startY + 2][targetX];
            let moveTwice = (targetY === startY + 2 && (t2 === null || t2[0] !== color));
            return moveOnce || moveTwice; 
        } 
        return moveOnce;
    }
}

function isValidRookMove(boardArray, color, startX, startY, targetX, targetY) {

    // if going to eat of the same color
    if (boardArray[targetY][targetX] != null && boardArray[targetY][targetX][0] === color) return false;
    // eat in horizontal or vertical lines, not of the same color
    return (startX === targetX || startY === targetY)
        && isPathClearForRook(boardArray, startX, startY, targetX, targetY);
}

function isPathClearForRook(boardArray, startX, startY, targetX, targetY) {
    // we already know its moving either horizontally or vertically
    // moving vertically
    if (startX === targetX) {
        const step = targetY > startY ? 1 : -1;
        for (let y = startY + step; y !== targetY; y += step) {
            if (boardArray[y][startX] !== null) {
                // path isn't empty
                return false;
            }
        }
    // moving horizontally
    } else if (startY === targetY) {
        const step = targetX > startX ? 1 : -1;
        for (let x = startX + step; x !== targetX; x += step) {
            if (boardArray[startY][x] !== null) {
                // path isn't empty
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}

function isValidKnightMove(boardArray, color, startX, startY, targetX, targetY) {
    const dx = Math.abs(startX - targetX);
    const dy = Math.abs(startY - targetY);
    // if correct direction, and not of the same color
    return ((dx === 2 && dy === 1) || (dx === 1 && dy === 2))
        && (boardArray[targetY][targetX] == null || boardArray[targetY][targetX][0] !== color);
}

function isValidBishopMove(boardArray, color, startX, startY, targetX, targetY) {
    // cant eat the same color
    if (boardArray[targetY][targetX] !== null && boardArray[targetY][targetX][0] === color) return false;

    // check it moves diagonally
    if (Math.abs(startX - targetX) === Math.abs(startY - targetY)) {
        return isPathClearForBishop(boardArray, startX, startY, targetX, targetY);
    }
    return false;
}

function isPathClearForBishop(boardArray, startX, startY, targetX, targetY) {

    let deltaX = targetX - startX;
    let deltaY = targetY - startY;

    // direction of movement
    const stepX = deltaX > 0 ? 1 : -1;
    const stepY = deltaY > 0 ? 1 : -1;

    // check that path is empty
    for (let i = 1; i < Math.abs(deltaX); i++) {
        const x = startX + (i * stepX);
        const y = startY + (i * stepY);
        if (boardArray[y][x] !== null) {
            // path isn't empty
            return false;
        }
    }
    return true;
}

function isValidQueenMove(boardArray, color, startX, startY, targetX, targetY) {

    if (boardArray[targetY][targetX] !== null && boardArray[targetY][targetX][0] === color) return false;
    // check horizontal or vertical move
    if (startX === targetX || startY === targetY) {
        return isValidRookMove(boardArray, color, startX, startY, targetX, targetY);
    // check for diagonal move
    } else if (Math.abs(startX - targetX) === Math.abs(startY - targetY)) {
        return isValidBishopMove(boardArray, color, startX, startY, targetX, targetY);
    }
    // not a valid move
    return false;
}

function isValidKingMove(boardArray, color, startX, startY, targetX, targetY) {
    
    if (boardArray[targetY][targetX] !== null && boardArray[targetY][targetX][0] === color) return false;
    // calculate travel
    const deltaX = Math.abs(startX - targetX);
    const deltaY = Math.abs(startY - targetY);
    if (deltaX == 0 && deltaY == 0) return false;
    // move one square
    if (deltaX <= 1 && deltaY <= 1) {
        // check for check
        return true;
    }
    return false;
}

export function isValidCastling(boardArray, color, startX, startY, targetX, targetY, whiteCastleLeft, whiteCastleRight, blackCastleLeft, blackCastleRight) {
    
    let deltaX = targetX - startX;
    let deltaY = targetY - startY;

    if (deltaY !== 0 || Math.abs(deltaX) !== 2) return false;

    // its like moving a rook from the king pos to one before the rook pos
    if (color == 'w') {
        // right castle
        if (deltaX > 0) {
            if (!whiteCastleRight) return false;
            return isPathClearForRook(boardArray, 4, 7, 6, 7);
        // left castle
        } else {
            if (!whiteCastleLeft) return false;
            return isPathClearForRook(boardArray, 4, 7, 1, 7);
        }
    // im not reversing the directions
    } else {
        // right castle
        if (deltaX > 0) {
            if (!blackCastleRight) return false;
            return isPathClearForRook(boardArray, 4, 0, 6, 0);
        // left castle
        } else {
            if (!blackCastleLeft) return false;
             return isPathClearForRook(boardArray, 4, 0, 1, 0);
        }
    }
}