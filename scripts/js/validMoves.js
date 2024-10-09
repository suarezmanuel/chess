import { whitePositions, blackPositions, deleteFromMap } from "./main.js";

// up right down left
let rookDirections  = [[0,-1], [1,0], [0,1], [-1,0]];
// up-right bottom-right bottom-left up-left
let bishopDirections = [[1,-1], [1,1], [-1,1], [-1,-1]];
// clock wise starting from up 
let knightDirections = [[1,-2], [2,-1], [2,1], [1,2], [-1,2], [-2,1], [-2,-1], [-1,-2]];
let kingDirections   = [[0,-1], [1,0], [0,1], [-1,0], [1,-1], [1,1], [-1,1], [-1,-1]];

export function getValidMovesWhite(board) {

    let moves = [];
    whitePositions.forEach((value, key) => {
        let a = getValidMoves(board, "w"+value, key[0], key[1]);
        moves.push.apply(moves, validMovesFromArray(board, key[0], key[1], a));
        // a.forEach(move => {
        //     let temp = board[move[1]][move[0]];
        //     // start
        //     board[key[1]][key[0]] = null;
        //     // target
        //     board[move[1]][move[0]] = "w"+value;
        //     if (value == 'k') kingPos = [move[0][move[1]]];

        //     if (!isInCheck(board, 'w', kingPos[0], kingPos[1])) {
        //         if (!moves.some(a => a[0] == move[0] && a[1] == move[1])) moves.push(move);
        //     }

        //     // start
        //     board[key[1]][key[0]]   = "w"+value;
        //     if (value == 'k') kingPos = [key[0][key[1]]];
        //     // end
        //     board[move[1]][move[0]] = temp;    
        // });
    });
    return moves;
}

export function getValidMovesBlack(board) {

    let moves = [];
    blackPositions.forEach((value, key) => {
        let a = getValidMoves(board, "b"+value, key[0], key[1]);
        moves.push.apply(moves, validMovesFromArray(board, key[0], key[1], a));
        // a.forEach(move => {
        //     let temp = board[move[1]][move[0]];
        //     // start
        //     board[key[1]][key[0]]   = null;
        //     // target
        //     board[move[1]][move[0]] = "b"+value;
        //     if (value == 'k') kingPos = [move[0][move[1]]];

        //     if (!isInCheck(board, 'b')) {
        //         if (!moves.some(a => a[0] == move[0] && a[1] == move[1])) moves.push(move);
        //     }
            
        //     // start
        //     board[key[1]][key[0]]   = "b"+value;
        //     if (value == 'k') kingPos = [key[0][key[1]]];
        //     // end
        //     board[move[1]][move[0]] = temp;    
        // });
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
        let temp = board[move[1]][move[0]];
        // start
        board[startY][startX]   = null;
        // target
        board[move[1]][move[0]] = color+piece;

        if (piece == 'k') kingPos = [move[0], move[1]];

        if (!isInCheck(board, color, kingPos)) {
            if (!moves.some(a => a[0] == move[0] && a[1] == move[1])) moves.push(move);
        }
        
        // start
        board[startY][startX] = color+piece;
        if (piece == 'k') kingPos = [startY, startX];
        // end
        board[move[1]][move[0]] = temp;   

        // if (piece == 'k') {
        //     if (color == 'w') {
        //         // whitePositions.set([move[0], move[1]], color);
        //         // deleteFromMap(whitePositions, [move[0], move[1]]);
        //         // if (temp) blackPositions.set([move[0], move[1]], temp.charAt(1));
        //         deleteFromMap(whitePositions, [move[0], move[1]]);
        //         whitePositions.set([startX, startY], piece);
        //         if (temp) blackPositions.set([move[0], move[1]], temp.charAt(1));
        //     } else {
        //         deleteFromMap(blackPositions, [move[0], move[1]]);
        //         blackPositions.set([startX, startY], piece);
        //         if (temp) whitePositions.set([move[0], move[1]], temp.charAt(1));
        //     }
        // }
    });
    return moves;
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
    if (piece === null) return false;
    return piece[0] === color;
}

export function checkPiece(piece1, piece2) {
    if (piece1 === null) return false;
    return piece1[1] === piece2;
}

// no en pessant yet
function pawnMoves(board, color, xPos, yPos) {

    let moves = [];
    let oppColor = (color === 'w') ? 'b' : 'w';
    // can eat of the opposite color
   
    if (yPos-1 >= 0) {
        if (xPos+1  < 8 && checkColor(board[yPos-1][xPos+1], oppColor)) moves.push([xPos+1, yPos-1]);
        if (xPos-1 >= 0 && checkColor(board[yPos-1][xPos-1], oppColor)) moves.push([xPos-1, yPos-1]);
    }
    
    if (color == 'w') {
         // can eat of the opposite color
        if (yPos-1 >= 0) {
            if (xPos+1  < 8 && checkColor(board[yPos-1][xPos+1], oppColor)) moves.push([xPos+1, yPos-1]);
            if (xPos-1 >= 0 && checkColor(board[yPos-1][xPos-1], oppColor)) moves.push([xPos-1, yPos-1]);
        }
        // move twice at start
        if (yPos == 6 && board[yPos-2][xPos] === null) moves.push([xPos, yPos-2]);
        // move once forward
        if (yPos > 0  && board[yPos-1][xPos] === null) moves.push([xPos, yPos-1]);
    // if black
    } else {
        // can eat of the opposite color
        if (yPos+1 < 8) {
            if (xPos+1  < 8 && checkColor(board[yPos+1][xPos+1], oppColor)) moves.push([xPos+1, yPos+1]);
            if (xPos-1 >= 0 && checkColor(board[yPos+1][xPos-1], oppColor)) moves.push([xPos-1, yPos+1]);
        }
        // move twice at start
        if (yPos == 1 && board[yPos+2][xPos] === null) moves.push([xPos, yPos+2]);
        // move once forward
        if (yPos <  7 && board[yPos+1][xPos] === null) moves.push([xPos, yPos+1]);
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
            moves.push([xTemp, yTemp]);
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
            moves.push([xTemp, yTemp]);
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
            moves.push([xTemp, yTemp]);
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
            moves.push([xTemp, yTemp]);
            // at first that can eat, continue
            if (checkColor(board[yTemp][xTemp], oppColor)) continue;
        }
    }
    return moves;
}

export function isInCheck(board, color, kingPos) {

    //let positions = color === "w" ? whitePositions : blackPositions;
    // let kingPos = [];
    // positions.forEach((value, key) => {
    //     if (value == 'k') kingPos = key;
    // });
    let xPos = kingPos[0];
    let yPos = kingPos[1];

    let oppColor = (color === 'w') ? 'b' : 'w';
    let _knightMoves = knightMoves(board, color, xPos, yPos);
    let _rookMoves   = rookMoves(board, color, xPos, yPos);
    let _bishopMoves = bishopMoves(board, color, xPos, yPos)
    let _kingMoves = kingMoves(board, color, xPos, yPos);
    let _pawnMoves;
    let _isInCheck = false;


    if (color == 'w') {
        _pawnMoves = [[xPos-1, yPos-1], [xPos+1, yPos-1]];
    } else {
        _pawnMoves = [[xPos-1, yPos+1], [xPos+1, yPos+1]];
    }

    _knightMoves.forEach(square => {
        if (checkColor(board[square[1]][square[0]], oppColor) && checkPiece(board[square[1]][square[0]], 'n')) {
            _isInCheck = true;
        }
     });

    if (_isInCheck) return true;

    _rookMoves.forEach(square => {
        if (checkColor(board[square[1]][square[0]], oppColor) && (checkPiece(board[square[1]][square[0]], 'r') || checkPiece(board[square[1]][square[0]], 'q'))) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;

    _bishopMoves.forEach(square => {
        if (checkColor(board[square[1]][square[0]], oppColor) && (checkPiece(board[square[1]][square[0]], 'b') || checkPiece(board[square[1]][square[0]], 'q'))) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;
    
    _kingMoves.forEach(square => {
        if (checkColor(board[square[1]][square[0]], oppColor) && checkPiece(board[square[1]][square[0]], 'k')) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;

    _pawnMoves.forEach(square => {
        if (square[0] >= 0 && square[0] < 8 && square[1] >= 0 && square[1] < 7
            && checkColor(board[square[1]][square[0]], oppColor) && checkPiece(board[square[1]][square[0]], 'p')) {
            _isInCheck = true;
        }
    });

    if (_isInCheck) return true;

    return false;
}
