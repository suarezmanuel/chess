import { getValidMovesWhite, getValidMovesBlack, isInCheck } from './public/js/validMoves.js';

let count = 0;

function perft (fen, depth) {
    let [board, turn, castling, enPassant, halfMoveClock, fullMoveNumber] = getBoardFromFen(fen);
    
    const g = {
      hasWhiteKingMoved: false,
      hasWhiteRightRookMoved: !castling.includes('K'),
      hasWhiteLeftRookMoved: !castling.includes('Q'),

      hasBlackKingMoved: false,
      hasBlackRightRookMoved: !castling.includes('k'),
      hasBlackLeftRookMoved: !castling.includes('q'),

      prevX: enPassant === '-' ? 0 : enPassant.charAt(0) - 'a',
      prevY: enPassant === '-' ? 0 : 8 - (enPassant.charAt(1) - '0')
    };


    let whitePositions = new Map();
    let blackPositions = new Map();

    for (let i=0; i<8; i++) {
        for (let j=0; j<8; j++) {
            if (board[j][i] === null) continue;
            let positions = (board[j][i].charAt(0) == 'w') ? whitePositions : blackPositions;
            positions.set([i, j], board[j][i].charAt(1));
        }
    }
    help(board, whitePositions, blackPositions, turn, ((turn =='w') ? 'b' : 'w'), depth, g);
}

let local_count = 0;
function help (board, whitePositions, blackPositions, turn, oppTurn, depth, g) {

    let possibleMoves = (turn === 'w') ? getValidMovesWhite(board, whitePositions, g) : getValidMovesBlack(board, blackPositions, g);

    if (possibleMoves.length == 0 || depth === 0) {
      local_count++;
      count++;
      return;
    }

    possibleMoves.forEach(move => {
        let eaten = board[move[3]][move[2]];
        let g_copy = {
          hasWhiteKingMoved: g.hasWhiteKingMoved,
          hasWhiteRightRookMoved: g.hasWhiteRightRookMoved,
          hasWhiteLeftRookMoved: g.hasWhiteLeftRookMoved,

          hasBlackKingMoved: g.hasBlackKingMoved,
          hasBlackRightRookMoved: g.hasBlackRightRookMoved,
          hasBlackLeftRookMoved: g.hasBlackLeftRookMoved,

          prevX: g.prevX,
          prevY: g.prevY
        };

        let pieceType = board[move[1]][move[0]].charAt(1);
        movePieceServer(board, move, whitePositions, blackPositions, g);

        help(board, whitePositions, blackPositions, oppTurn, turn, depth-1, g);
        if (depth == 1) {
          //console.log(`${String.fromCharCode(97 + +move[0])}${8 - +move[1]}${String.fromCharCode(97 + +move[2])}${8 - +move[3]}${move[4] ? move[4] : ''}: ${local_count}`);
          local_count = 0;
        }
        // undo move
        g = g_copy;
        undoMoveServer(board, move[0], move[1], move[2], move[3], whitePositions, blackPositions, g, eaten, pieceType)
    });
}

perft('rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8', 4);
// perft('rnbq1k1r/pp1Pbppp/2p4B/8/2B5/8/PPP1NnPP/RN1QK2R b KQ - 1 8', 1);
console.log(count);

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


export function movePieceServer(boardArray, move, whitePositions, blackPositions, g) {
 
  let startX = move[0];
  let startY = move[1];
  let targetX = move[2];
  let targetY = move[3];

  let pieceStr = boardArray[startY][startX];
  let piece = pieceStr.charAt(1);
  let color = pieceStr.charAt(0);

  // Pawn promotion
  if (piece === 'p' && (targetY === 0 || targetY === 7)) {
    piece = move[4];
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

  g.prevX = targetX;
  g.prevY = targetY;

  let positions = color === 'b' ? whitePositions : blackPositions;
  let kingPos = [];
  positions.forEach((value, key) => {
    if (value === 'k') kingPos = key;
  });


}

export function undoMoveServer(boardArray, startX, startY, targetX, targetY, whitePositions, blackPositions, g, eaten, pieceType) {
 
  let pieceStr = boardArray[targetY][targetX];
  let piece = pieceStr.charAt(1);
  let color = pieceStr.charAt(0);

  // Pawn promotion
  if (pieceType === 'p' && piece !== 'p') {
    piece = 'p';
    pieceStr = color + piece;
  }

  // En passant
  if (piece === 'p') {
    let y = targetY + (color === 'w' ? 1 : -1);
    if (Math.abs(targetX - startX) === 1 && eaten === null) {
      boardArray[targetY + (color === 'w' ? 1 : -1)][targetX] = color === 'w' ? "bp" : "wp";
      let positions = color === 'b' ? whitePositions : blackPositions;
      positions.set([targetX, y], 'p');
    }
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
      deleteFromMap(whitePositions, [rookTargetX, rookTargetY]);
      whitePositions.set([rookStartX, rookStartY], 'r');
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
      deleteFromMap(blackPositions, [rookTargetX, rookTargetY]);
      blackPositions.set([rookStartX, rookStartY], 'r');
    }
    boardArray[rookStartY][rookStartX] = rookStr;
    boardArray[rookTargetY][rookTargetX] = null;
  }

  if (color === 'w') {
    whitePositions.set([startX, startY], piece);
    deleteFromMap(whitePositions, [targetX, targetY]);
    if (eaten) blackPositions.set([targetX, targetY], eaten.charAt(1));
  } else {
    blackPositions.set([startX, startY], piece);
    deleteFromMap(blackPositions, [targetX, targetY]);
    if (eaten) whitePositions.set([targetX, targetY], eaten.charAt(1));
  }

  // Update the board array
  boardArray[startY][startX] = pieceStr;
  boardArray[targetY][targetX] = eaten;
}


export function deleteFromMap(map, arr) {
    for (let key of map.keys()) {
        if (JSON.stringify(key) === JSON.stringify(arr)) {
          map.delete(key);
          break;
        }
    }
}