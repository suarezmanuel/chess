import { io } from 'https://cdn.socket.io/4.5.4/socket.io.esm.min.js';

export function connectBot() {
    const socket = io();
    socket.emit('joinRoom', 807);
    
    let color = null;
    
    socket.on('playerColor', (data) => {
        color = data.color;
        console.log(`bot got color: ${color}`);
    });
    // socket.on('positions', (data) => {
    
    // })
    socket.on('receivePossibleMoves', (data) => {
        let moves = data.possibleMoves;
        let move = moves[Math.floor(Math.random()*moves.length)];
        if (move) {
            socket.emit('moveMade', { 
                startX: move[0], 
                startY: move[1], 
                targetX: move[2], 
                targetY: move[3], 
                playerColor: color 
            });
        }
    });
};

