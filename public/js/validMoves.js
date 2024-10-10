export let whitePositions = new Map([
    [[0, 6], 'p'], [[1, 6], 'p'], [[2, 6], 'p'], [[3, 6], 'p'],
    [[4, 6], 'p'], [[5, 6], 'p'], [[6, 6], 'p'], [[7, 6], 'p'],
    [[0, 7], 'r'], [[1, 7], 'n'], [[2, 7], 'b'], [[3, 7], 'q'],
    [[4, 7], 'k'], [[5, 7], 'b'], [[6, 7], 'n'], [[7, 7], 'r']
]);

export let blackPositions = new Map([
    [[0, 1], 'p'], [[1, 1], 'p'], [[2, 1], 'p'], [[3, 1], 'p'],
    [[4, 1], 'p'], [[5, 1], 'p'], [[6, 1], 'p'], [[7, 1], 'p'],
    [[0, 0], 'r'], [[1, 0], 'n'], [[2, 0], 'b'], [[3, 0], 'q'],
    [[4, 0], 'k'], [[5, 0], 'b'], [[6, 0], 'n'], [[7, 0], 'r']
]);

export const g = {
  hasWhiteKingMoved: false,
  hasWhiteRightRookMoved: false,
  hasWhiteLeftRookMoved: false,

  hasBlackKingMoved: false,
  hasBlackRightRookMoved: false,
  hasBlackLeftRookMoved: false,

  prevX: null,
  prevY: null,
};


// export const whitePositions = new Map([[[0,6], 'p'], [[1,6], 'p'], [[2,6], 'p'], [[3,6], 'p'], [[4,6], 'p'], [[5,6], 'p'], [[6,6], 'p'], [[7,6], 'p'], 
//     [[0,7], 'r'], [[1,7], 'n'], [[2,7], 'b'], [[3,7], 'q'], [[4,7], 'k'], [[5,7], 'b'], [[6,7], 'n'], [[7,7], 'r']]);

// export const blackPositions = new Map([[[0,1], 'p'], [[1,1], 'p'], [[2,1], 'p'], [[3,1], 'p'], [[4,1], 'p'], [[5,1], 'p'], [[6,1], 'p'], [[7,1], 'p'], 
//     [[0,0], 'r'], [[1,0], 'n'], [[2,0], 'b'], [[3,0], 'q'], [[4,0], 'k'], [[5,0], 'b'], [[6,0], 'n'], [[7,0], 'r']]);


// up right down left
let rookDirections  = [[0,-1], [1,0], [0,1], [-1,0]];
// up-right bottom-right bottom-left up-left
let bishopDirections = [[1,-1], [1,1], [-1,1], [-1,-1]];
// clock wise starting from up 
let knightDirections = [[1,-2], [2,-1], [2,1], [1,2], [-1,2], [-2,1], [-2,-1], [-1,-2]];
let kingDirections   = [[0,-1], [1,0], [0,1], [-1,0], [1,-1], [1,1], [-1,1], [-1,-1]];

// export function getValidMovesWhite(board) {

//     let moves = [];
//     whitePositions.forEach((value, key) => {
//         let a = getValidMoves(board, "w"+value, key[0], key[1]);
//         moves.push.apply(moves, validMovesFromArray(board, key[0], key[1], a));
//     });
//     return moves;
// }

export function getValidMovesWhite(board) {

    let moves = [];
    whitePositions.forEach((value, key) => {
        let a = getValidMoves(board, "w"+value, key[0], key[1], g.prevX, g.prevY);
        moves.push.apply(moves, validMovesFromArray(board, key[0], key[1], a));
    });
    return moves;
}

export function getValidMovesBlack(board) {

    let moves = [];
    blackPositions.forEach((value, key) => {
        let a = getValidMoves(board, "b"+value, key[0], key[1], g.prevX, g.prevY);
        moves.push.apply(moves, validMovesFromArray(board, key[0], key[1], a));
    });
    return moves;
}

export function validMovesFromArray(board, startX, startY, array) {
    
    let color = board[startY][startX].charAt(0);
    let piece = board[startY][startX].charAt(1);
    let positions = color === "w" ? whitePositions : blackPositions;
    let kingPos = [];
    positions.forEach((value, key) => {
        if (value == 'k') kingPos = key;
    });
    let moves = [];

    array.forEach(move => {
        let temp = board[move[3]][move[2]];
        // start
        board[startY][startX]   = null;
        // target
        board[move[3]][move[2]] = color+piece;

        if (piece == 'k') kingPos = [move[2], move[3]];

        if (!isInCheck(board, color, kingPos)) {
            if (!moves.some(a => a[0] == move[2] && a[1] == move[3])) moves.push(move);
        }
        
        // start
        board[startY][startX] = color+piece;
        if (piece == 'k') kingPos = [startY, startX];
        // end
        board[move[3]][move[2]] = temp;   
    });
    return moves;
}

function isBad(a) {
    return (a === undefined || a === null);
}

export function getValidMoves (board, piece, xPos, yPos) {

    const pieceType = piece.substring(1); // e.g., 'p' for pawn, 'r' for rook, etc.
    let moves = [];
    // need to account for taking pieces
    switch (pieceType) {
        case 'p': // Pawn move (basic example)
            moves = validMovesFromArray(board, xPos, yPos, pawnMoves(board, piece[0], xPos, yPos));
            break;
        case 'r': // Rook move
            moves = validMovesFromArray(board, xPos, yPos, rookMoves(board, piece[0], xPos, yPos));
            break;
        case 'n': // Knight move
            moves = validMovesFromArray(board, xPos, yPos, knightMoves(board, piece[0], xPos, yPos));
            break;
        case 'b': // Bishop move
            moves = validMovesFromArray(board, xPos, yPos, bishopMoves(board, piece[0], xPos, yPos));
            break;
        case 'q': // Queen move
            moves = validMovesFromArray(board, xPos, yPos, queenMoves(board, piece[0], xPos, yPos));
            break;
        case 'k': // King move
            moves = validMovesFromArray(board, xPos, yPos, kingMoves(board, piece[0], xPos, yPos));
            break;
    }

    return moves;
}

export function checkColor(piece, color) {
    if (isBad(piece)) return false;
    return piece[0] === color;
}

export function checkPiece(piece1, piece2) {
    if (isBad(piece1)) return false;
    return piece1[1] === piece2;
}

// no en pessant yet
function pawnMoves(board, color, xPos, yPos) {

    let moves = [];
    let oppColor = (color === 'w') ? 'b' : 'w';
    // can eat of the opposite color
    
    if (yPos-1 >= 0) {
        if (color === 'w') {
            if (xPos+1  < 8 && checkColor(board[yPos-1][xPos+1], oppColor)) moves.push([xPos, yPos, xPos+1, yPos-1]);
            if (xPos-1 >= 0 && checkColor(board[yPos-1][xPos-1], oppColor)) moves.push([xPos, yPos, xPos-1, yPos-1]);
        } else {
            if (xPos+1  < 8 && checkColor(board[yPos+1][xPos+1], oppColor)) moves.push([xPos, yPos, xPos+1, yPos+1]);
            if (xPos-1 >= 0 && checkColor(board[yPos+1][xPos-1], oppColor)) moves.push([xPos, yPos, xPos-1, yPos+1]);
        }
    }
    
    // en passant
    if (!isBad(g.prevX) && !isBad(g.prevY) && xPos > 0 && xPos < 7 && checkPiece(board[g.prevY][g.prevX], 'p')) {
        if (color == 'w') {
            if (yPos == 3 && g.prevY == 3) {
                // prev is last target X
                if (g.prevX == xPos-1 || g.prevX == xPos+1)
                    moves.push([xPos, yPos, g.prevX, yPos-1]);
            }
        } else {
            if (yPos == 4 && g.prevY == 4) {
                if (g.prevX == xPos-1 || g.prevX == xPos+1)
                    moves.push([xPos, yPos, g.prevX, yPos+1]);
            }
        }
    }

    if (color == 'w') {
         // can eat of the opposite color
        if (yPos-1 >= 0) {
            if (xPos+1  < 8 && checkColor(board[yPos-1][xPos+1], oppColor)) moves.push([xPos, yPos, xPos+1, yPos-1]);
            if (xPos-1 >= 0 && checkColor(board[yPos-1][xPos-1], oppColor)) moves.push([xPos, yPos, xPos-1, yPos-1]);
        }
        // move twice at start
        if (yPos == 6 && isBad(board[yPos-2][xPos])) moves.push([xPos, yPos, xPos, yPos-2]);
        // move once forward
        if (yPos > 0  && isBad(board[yPos-1][xPos])) moves.push([xPos, yPos, xPos, yPos-1]);
    // if black
    } else {
        // can eat of the opposite color
        if (yPos+1 < 8) {
            if (xPos+1  < 8 && checkColor(board[yPos+1][xPos+1], oppColor)) moves.push([xPos, yPos, xPos+1, yPos+1]);
            if (xPos-1 >= 0 && checkColor(board[yPos+1][xPos-1], oppColor)) moves.push([xPos, yPos, xPos-1, yPos+1]);
        }
        // move twice at start
        if (yPos == 1 && isBad(board[yPos+2][xPos])) moves.push([xPos, yPos, xPos, yPos+2]);
        // move once forward
        if (yPos <  7 && isBad(board[yPos+1][xPos])) moves.push([xPos, yPos, xPos, yPos+1]);
    }

    return moves;
}

function rookMoves(board, color, xPos, yPos) {
    let moves = [];
    let oppColor = (color === 'w') ? 'b' : 'w';
    let xTemp;
    let yTemp;
    for (let i=0; i < rookDirections.length; i++) {
        xTemp = xPos;
        yTemp = yPos;
        while (xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8) {
            xTemp += rookDirections[i][0];
            yTemp += rookDirections[i][1];
            if (!(xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8)) break;
            // stop if same color
            if (checkColor(board[yTemp][xTemp], color)) break;
            moves.push([xPos, yPos, xTemp, yTemp]);
            // at first that can eat, break
            if (checkColor(board[yTemp][xTemp], oppColor)) break;
        }
    }
    return moves;
}

function knightMoves(board, color, xPos, yPos) {
    let moves = [];
    let oppColor = (color === 'w') ? 'b' : 'w';
    let xTemp;
    let yTemp;
    for (let i=0; i < knightDirections.length; i++) {
        xTemp = xPos;
        yTemp = yPos;
        if (xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8) {
            xTemp += knightDirections[i][0];
            yTemp += knightDirections[i][1];
            if (!(xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8)) continue;
            // stop if same color
            if (checkColor(board[yTemp][xTemp], color)) continue;
            moves.push([xPos, yPos, xTemp, yTemp]);
            // at first that can eat, continue
            if (checkColor(board[yTemp][xTemp], oppColor)) continue;
        }
    }
    return moves;
}

function bishopMoves(board, color, xPos, yPos) {
    let moves = [];
    let oppColor = (color === 'w') ? 'b' : 'w';
    let xTemp;
    let yTemp;
    for (let i=0; i < bishopDirections.length; i++) {
        xTemp = xPos;
        yTemp = yPos;
        while (xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8) {
            xTemp += bishopDirections[i][0];
            yTemp += bishopDirections[i][1];
            if (!(xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8)) break;
            // stop if same color
            if (checkColor(board[yTemp][xTemp], color)) break;
            moves.push([xPos, yPos, xTemp, yTemp]);
            // at first that can eat, break
            if (checkColor(board[yTemp][xTemp], oppColor)) break;
        }
    }
    return moves;
}

function queenMoves(board, color, xPos, yPos) {
    let moves = [];
    moves.push.apply(moves, bishopMoves(board, color, xPos, yPos));
    moves.push.apply(moves, rookMoves(board, color, xPos, yPos));
    return moves;
}

function kingMoves(board, color, xPos, yPos) {
    let moves = [];
    let oppColor = (color === 'w') ? 'b' : 'w';
    let xTemp;
    let yTemp;
    for (let i=0; i < kingDirections.length; i++) {
        xTemp = xPos;
        yTemp = yPos;
        if (xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8) {
            xTemp += kingDirections[i][0];
            yTemp += kingDirections[i][1];
            if (!(xTemp >= 0 && xTemp < 8 && yTemp >= 0 && yTemp < 8)) continue;
            // stop if same color
            if (checkColor(board[yTemp][xTemp], color)) continue;
            moves.push([xPos, yPos, xTemp, yTemp]);
            // at first that can eat, continue
            if (checkColor(board[yTemp][xTemp], oppColor)) continue;
        }
    }
    return [...moves, ...castleMoves(board, color)];
}

function castleMoves(board, color) {
    // let moves = [];
    // let hasKingMoved = color === 'w' ? hasWhiteKingMoved : hasBlackKingMoved;
    // if (hasKingMoved || isInCheck(board, color, [4,color === 'w' ? 7 : 0])) return moves;
    // if (color === 'w') {
    //     if (!hasWhiteLeftRookMoved) {
    //         if (board[7][0] === "wr" && isBad(board[7][1]) && isBad(board[7][2]) && isBad(board[7][3]) && !isInCheck(board, color, [7, 3])) moves.push([4, 7, 2, 7]);
    //     }
    //     if (!hasWhiteRightRookMoved) {
    //         if (board[7][7] === "wr" && isBad(board[7][5]) && isBad(board[7][6]) && !isInCheck(board, color, [7, 5])) moves.push([4, 7, 6, 7]);
    //     }
    // } else {
    //     if (!hasBlackLeftRookMoved) {
    //         if (board[0][0] === "br" && isBad(board[0][1]) && isBad(board[0][2]) && isBad(board[0][3]) && !isInCheck(board, color, [0, 3])) moves.push([4, 0, 2, 0]);
    //     }
    //     if (!hasBlackRightRookMoved) {
    //         if (board[0][7] === "br" && isBad(board[0][5]) && isBad(board[0][6]) && !isInCheck(board, color, [0, 5])) moves.push([4, 0, 6, 0]);
    //     }
    // }
    // return moves;
    let moves = [];
    let hasKingMoved = color === 'w' ? g.hasWhiteKingMoved : g.hasBlackKingMoved;
    if (hasKingMoved) return moves;
    if (color === 'w') {
        if (!g.hasWhiteLeftRookMoved) {
            if (board[7][0] === "wr" && isBad(board[7][1]) && isBad(board[7][2]) && isBad(board[7][3])) moves.push([4, 7, 2, 7]);
        }
        if (!g.hasWhiteRightRookMoved) {
            if (board[7][7] === "wr" && isBad(board[7][5]) && isBad(board[7][6])) moves.push([4, 7, 6, 7]);
        }
    } else {
        if (!g.hasBlackLeftRookMoved) {
            if (board[0][0] === "br" && isBad(board[0][1]) && isBad(board[0][2]) && isBad(board[0][3])) moves.push([4, 0, 2, 0]);
        }
        if (!g.hasBlackRightRookMoved) {
            if (board[0][7] === "br" && isBad(board[0][5]) && isBad(board[0][6])) moves.push([4, 0, 6, 0]);
        }
    }
    return moves;
}

export function isInCheck(board, color, kingPos) {

    let xPos = kingPos[0];
    let yPos = kingPos[1];

    let oppColor = (color === 'w') ? 'b' : 'w';
    let _knightMoves = knightMoves(board, color, xPos, yPos);
    let _rookMoves   = rookMoves(board, color, xPos, yPos);
    let _bishopMoves = bishopMoves(board, color, xPos, yPos)
    let _kingMoves   = kingMoves(board, color, xPos, yPos);
    let _pawnMoves;
    let _isInCheck = false;


    if (color == 'w') {
        _pawnMoves = [[xPos-1, yPos-1], [xPos+1, yPos-1]];
    } else {
        _pawnMoves = [[xPos-1, yPos+1], [xPos+1, yPos+1]];
    }

    _knightMoves.forEach(square => {
        if (checkColor(board[square[3]][square[2]], oppColor) && checkPiece(board[square[3]][square[2]], 'n')) {
            _isInCheck = true;
        }
     });

    if (_isInCheck) return true;

    _rookMoves.forEach(square => {
        if (checkColor(board[square[3]][square[2]], oppColor) && (checkPiece(board[square[3]][square[2]], 'r') || checkPiece(board[square[3]][square[2]], 'q'))) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;

    _bishopMoves.forEach(square => {
        if (checkColor(board[square[3]][square[2]], oppColor) && (checkPiece(board[square[3]][square[2]], 'b') || checkPiece(board[square[3]][square[2]], 'q'))) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;
    
    _kingMoves.forEach(square => {
        if (checkColor(board[square[3]][square[2]], oppColor) && checkPiece(board[square[3]][square[2]], 'k')) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;

    _pawnMoves.forEach(square => {
        if (square[2] >= 0 && square[2] < 8 && square[3] >= 0 && square[3] < 7
            && checkColor(board[square[3]][square[2]], oppColor) && checkPiece(board[square[3]][square[2]], 'p')) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;

    return false;
}
