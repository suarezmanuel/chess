// up right down left
let rookDirections = [[0,-1], [1,0], [0,1], [-1,0]];
// up-right bottom-right bottom-left up-left
let bishopDirections = [[1,-1], [1,1], [-1,1], [-1,-1]];
// clock wise starting from up 
let knightDirections = [[1,-2], [2,-1], [2,1], [1,2], [-1,2], [-2,1], [-2,-1], [-1,-2]];
let kingDirections = [[0,-1], [1,0], [0,1], [-1,0], [1,-1], [1,1], [-1,1], [-1,-1]];

let validWhiteMoves = [];
let validBlackMoves = [];

export function getValidMovesWhite() {

}

export function getValidMovesBlack() {

}

export function getValidMoves (board, piece, xPos, yPos, onCheck) {

    const pieceType = piece.substring(1); // e.g., 'p' for pawn, 'r' for rook, etc.
    let moves = [];
    // need to account for taking pieces
    switch (pieceType) {
        case 'p': // Pawn move (basic example)
            moves = pawnMoves(board, piece[0], xPos, yPos);
            break;
        case 'r': // Rook move
            moves = rookMoves(board, piece[0], xPos, yPos);
            break;
        case 'n': // Knight move
            moves = knightMoves(board, piece[0], xPos, yPos);
            break;
        case 'b': // Bishop move
            moves = bishopMoves(board, piece[0], xPos, yPos);
            break;
        case 'q': // Queen move
            moves = queenMoves(board, piece[0], xPos, yPos);
            break;
        case 'k': // King move
            moves = kingMoves(board, piece[0], xPos, yPos);
            break;
    }

    if (onCheck) {
        let ans = [];
        // return intersection between color's valid moves and moves
        return moves;
    }
    return moves;
}

export function checkColor(piece, color) {
    if (piece === null) return false;
    return piece[0] === color;
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
