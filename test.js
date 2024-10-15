import { getValidMovesWhite, getValidMovesBlack } from './public/js/validMoves.js';

let prevX;
let prevY;
let count = 0;

function perft (fen, depth) {
    let [board, turn, tmp1, tmp2, tmp4, tmp5] = getBoardFromFen(fen);
    let whitePositions = new Map();
    let blackPositions = new Map();

    for (let i=0; i<8; i++) {
        for (let j=0; j<8; j++) {
            if (board[i][j] === null) continue;
            let positions = (board[i][j].charAt(0) == 'w') ? whitePositions : blackPositions;
            positions.set([i, j], board[i][j].charAt(1));
        }
    }
    help(board, whitePositions, blackPositions, turn, ((turn =='w') ? 'b' : 'w'));

    return count;
}

function help (board, whitePositions, blackPositions, turn, oppTurn, depth) {

    let possibleMoves = (turn === 'w') ? getValidMovesWhite(board, prevX, prevY, whitePositions, blackPositions) : getValidMovesBlack(board, prevX, prevY, whitePositions, blackPositions);

    if (possibleMoves.length == 0 || depth === 0) return;

    let positions = (turn == 'w') ? whitePositions : blackPositions;
    let oppPositions = (turn == 'w') ? blackPositions : whitePositions;

    possibleMoves.forEach(move => {
        count++;
        let eaten = board[move[3]][move[2]];

        movePieceServer(board, move[0], move[1], move[2], move[3], whitePositions, blackPositions, prevX, prevY);

        help(board, whitePositions, blackPositions, oppTurn, turn, depth-1);

        // undo move
        deleteFromMap(positions, [move[2], move[3]]);
        positions.set([move[0], move[1]], board[move[3]][move[2]].charAt(1));

        board[move[1]][move[0]] = board[move[3]][move[2]];
        board[move[3]][move[2]] = eaten;

        if (eaten) oppPositions.set([move[2], move[3]], eaten.charAt(1));
    })
}

await new Promise(() => {}, 2000);
console.log(perft('rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8', 3));

function getBoardFromFen(fen) {
    let board = [
        [null, null, null, null, null, null, null, null],
        [null, null, null, null, null, null, null, null],
        [null, null, null, null, null, null, null, null],
        [null, null, null, null, null, null, null, null],
        [null, null, null, null, null, null, null, null],
        [null, null, null, null, null, null, null, null],
        [null, null, null, null, null, null, null, null],
        [null, null, null, null, null, null, null, null]
    ];

    const [position, turn, castling, enPassant, halfMoveClock, fullMoveNumber] = fen.split(' ');

    let idx = 0;
    for (let i = 0; i < position.length; i++) {
        let c = position.charAt(i);
        if (c > '0' && c <= '9') {
            idx += c - '0';
        } else {
            if (c >= 'A' && c <= 'Z') {
                board[Math.floor(idx / 8)][idx % 8] = `w${c.toLowerCase()}`;
                idx++;
            } else if (c >= 'a' && c <= 'z'){
                board[Math.floor(idx / 8)][idx % 8] = `b${c}`;
                idx++;
            }
        }
    }
    return [board, turn, castling, enPassant, halfMoveClock, fullMoveNumber];      
}


export function movePieceServer(boardArray, startX, startY, targetX, targetY, whitePositions, blackPositions, prevX, prevY) {
 
  let pieceStr = boardArray[startY][startX];
  let piece = pieceStr.charAt(1);
  let color = pieceStr.charAt(0);

  // Pawn promotion
  if (piece === 'p' && (targetY === 0 || targetY === 7)) {
    piece = 'q';
    pieceStr = color + piece;
  }

  // En passant
  if (piece === 'p') {
    let y = targetY + (color === 'w' ? 1 : -1);
    if (Math.abs(targetX - startX) === 1 && boardArray[targetY][targetX] === null) {
      boardArray[targetY + (color === 'w' ? 1 : -1)][targetX] = null;
      let positions = color === 'b' ? whitePositions : blackPositions;
      deleteFromMap(positions, [targetX, y]);
    }
  }

  // Update castling parameters
  if (piece === 'k') {
      if (color === 'w')
          g.hasWhiteKingMoved = true;
      else
          g.hasBlackKingMoved = true;
  } else if (piece === 'r') {
      if (startX === 0 && startY === 0)
          g.hasBlackLeftRookMoved = true;
      else if (startX === 0 && startY === 7)
          g.hasWhiteLeftRookMoved = true;
      else if (startX === 7 && startY === 0)
          g.hasBlackRightRookMoved = true;
      else if (startX === 7 && startY === 7)
          g.hasWhiteRightRookMoved = true;
  }

  // Check for castle
  if (piece === 'k' && Math.abs(targetX - startX) > 1) {
    let rookStartX, rookStartY, rookTargetX, rookTargetY, rookStr;
    if (color === 'w') {
      if (targetX === 6) {
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
      rookStr = 'wr';
      deleteFromMap(whitePositions, [rookStartX, rookStartY]);
      whitePositions.set([rookTargetX, rookStartY], 'r');
    } else {
      if (targetX === 6) {
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
      rookStr = 'br';
      deleteFromMap(blackPositions, [rookStartX, rookStartY]);
      blackPositions.set([rookTargetX, rookTargetY], 'r');
    }
    boardArray[rookTargetY][rookTargetX] = rookStr;
    boardArray[rookStartY][rookStartX] = null;
  }

  if (color === 'w') {
    deleteFromMap(whitePositions, [startX, startY]);
    whitePositions.set([targetX, targetY], piece);
    deleteFromMap(blackPositions, [targetX, targetY]);
  } else {
    deleteFromMap(blackPositions, [startX, startY]);
    blackPositions.set([targetX, targetY], piece);
    deleteFromMap(whitePositions, [targetX, targetY]);
  }

  // Update the board array
  boardArray[targetY][targetX] = pieceStr;
  boardArray[startY][startX] = null;

  prevX = targetX;
  prevY = targetY;

  let positions = color === 'b' ? whitePositions : blackPositions;
  let kingPos = [];
  positions.forEach((value, key) => {
    if (value === 'k') kingPos = key;
  });

  let opponentColor = color === 'w' ? 'b' : 'w';

  let onCheck = isInCheck(boardArray, opponentColor, kingPos);

  // Check if we put the opponent in checkmate or stalemate
  let allMoves;
  if (opponentColor === 'w') {
    allMoves = getValidMovesWhite(boardArray, prevX, prevY);
  } else {
    allMoves = getValidMovesBlack(boardArray, prevX, prevY);
  }
  if (allMoves.length === 0) {
    if (onCheck) {
      gameResult = {
        winner: color === 'w' ? 'white' : 'black',
        message: 'Checkmate',
      };
      console.log(`${gameResult.message}, ${gameResult.winner} wins`);
    } else {
      gameResult = {
        winner: 'none',
        message: 'Stalemate',
      };
      console.log(gameResult.message);
    }
    gameEnded = true;
  }
}

export function deleteFromMap(map, arr) {
    for (let key of map.keys()) {
        if (JSON.stringify(key) === JSON.stringify(arr)) {
        map.delete(key);
        break;
        }
    }
}