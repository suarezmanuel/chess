import express from 'express';
import http from 'http';
import { Server } from 'socket.io'; // Import the Socket.IO server
import { fileURLToPath } from 'url';
import path, { dirname } from 'path';

// Import your chess logic functions
import {
  getValidMoves,
  getValidMovesWhite,
  getValidMovesBlack,
  isInCheck,
  whitePositions,
  blackPositions,
  g
} from './public/js/validMoves.js';


// hasWhiteKingMoved,

// hasWhiteLeftRookMoved,
// hasWhiteRightRookMoved,
// hasBlackKingMoved,
// hasBlackLeftRookMoved,

// hasBlackRightRookMoved,
// whitePositions,
// blackPositions,
// prevX,

// prevY

// For __dirname in ES Modules
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Create Express app
const app = express();
const server = http.createServer(app);
const io = new Server(server); // Initialize the Socket.IO server

// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// Serve 'game.html' when the user accesses '/game'
app.get('/game', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'game.html'));
});

// Start the server
const port = process.env.PORT || 3000;
server.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});

// Game state variables
let boardArray = [
  ['br', 'bn', 'bb', 'bq', 'bk', 'bb', 'bn', 'br'],
  ['bp', 'bp', 'bp', 'bp', 'bp', 'bp', 'bp', 'bp'],
  [null, null, null, null, null, null, null, null],
  [null, null, null, null, null, null, null, null],
  [null, null, null, null, null, null, null, null],
  [null, null, null, null, null, null, null, null],
  ['wp', 'wp', 'wp', 'wp', 'wp', 'wp', 'wp', 'wp'],
  ['wr', 'wn', 'wb', 'wq', 'wk', 'wb', 'wn', 'wr'],
];

let currentTurn = 'w';
let gameEnded = false;
let gameResult = null;
let possibleMoves = [];

// Players
const players = {};

// Handle socket connections
io.on('connection', (socket) => {
  console.log('New user connected');

  let playerColor = null;

  // Assign player color
  if (!players.white) {
    players.white = socket;
    playerColor = 'w';
    socket.emit('playerColor', { color: 'w' });
  } else if (!players.black) {
    players.black = socket;
    playerColor = 'b';
    socket.emit('playerColor', { color: 'b' });
  } else {
    socket.emit('message', 'Game is full');
    socket.disconnect();
    return;
  }

  if (currentTurn === 'w' && players.white) {
    possibleMoves = getValidMovesWhite(boardArray);
    players.white.emit('receivePossibleMoves', { possibleMoves });
  } else if (currentTurn === 'b' && players.black) {
    possibleMoves = getValidMovesBlack(boardArray);
    players.black.emit('receivePossibleMoves', { possibleMoves });
  }

  // Handle 'moveMade' event
  socket.on('moveMade', (messageData) => {
    if (gameEnded) {
      socket.emit('gameEnded', { ...gameResult });
      return;
    }

    const { startX, startY, targetX, targetY, playerColor: movePlayerColor } = messageData;

    // Validate turn
    if (currentTurn !== movePlayerColor || playerColor !== movePlayerColor) {
      socket.emit('moveValidation', { response: 'invalid move', reason: 'Not your turn' });
      return;
    }

    // Validate turn
    if (!boardArray[startY][startX]) {
      socket.emit('moveValidation', { response: 'invalid move', reason: 'not a valid piece' });
      return;
    }

    // Validate move
    const isValidMove = possibleMoves.some(
      (move) =>
        move[0] === startX &&
        move[1] === startY &&
        move[2] === targetX &&
        move[3] === targetY
    );

    if (isValidMove) {
      // Perform the move
      movePieceServer(startX, startY, targetX, targetY);

      // Switch turns
      currentTurn = currentTurn === 'w' ? 'b' : 'w';

      // Send the move to all clients
      io.emit('moveMade', { startX, startY, targetX, targetY, playerColor });

      socket.emit('moveValidation', { response: 'valid move' });

      if (gameEnded) {
        // Notify both players of the game result
        io.emit('gameEnded', { ...gameResult });
        return;
      }

      // Send possible moves to the next player
      if (currentTurn === 'w' && players.white) {
        possibleMoves = getValidMovesWhite(boardArray);
        players.white.emit('receivePossibleMoves', { possibleMoves });
      } else if (currentTurn === 'b' && players.black) {
        possibleMoves = getValidMovesBlack(boardArray);
        players.black.emit('receivePossibleMoves', { possibleMoves });
      }
    } else {
      socket.emit('moveValidation', { response: 'invalid move', reason: 'Invalid move' });
    }
  });

  // Handle disconnection
  socket.on('disconnect', () => {
    console.log('User disconnected');
    if (playerColor === 'w') {
      delete players.white;
    } else if (playerColor === 'b') {
      delete players.black;
    }
  });
});

// Function to delete from positions map
function deleteFromMap(map, arr) {
    for (let key of map.keys()) {
      if (JSON.stringify(key) === JSON.stringify(arr)) {
        map.delete(key);
        break;
      }
    }
  }
  
// Function to perform the move on the server
function movePieceServer(startX, startY, targetX, targetY) {
  // if (!boardArray[startY][startX]) return false;

  // let moves = getValidMoves(boardArray, boardArray[startY][startX], startX, startY, prevX, prevY);

  // if (moves.some((move) => move[2] === targetX && move[3] === targetY)) {
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
        whitePositions.set(`${rookTargetX},${rookTargetY}`, 'r');
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

    let opponentColor = color === 'w' ? 'b' : 'w';

    let onCheck = isInCheck(boardArray, opponentColor, kingPos);

    // Check if we put the opponent in checkmate or stalemate
    let allMoves;
    if (opponentColor === 'w') {
      allMoves = getValidMovesWhite(boardArray);
    } else {
      allMoves = getValidMovesBlack(boardArray);
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
  // } else {
  //   console.log('Invalid move!');
  // }
}
