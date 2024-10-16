const createModule = require('./bin/temp.js');
const readline = require('readline');

(async () => {
    const Module = await createModule();

    const setBoardFromFen = Module.cwrap('setBoardFromFen', null, ['number', 'string', 'number', 'number', 'number', 'number', 'number']);

    const ptrSize = Module.HEAPU32.BYTES_PER_ELEMENT;
    const boardSize = 8 * 8 * ptrSize;
    const boardPtr = Module._malloc(boardSize);

    const boardHeap = new Uint32Array(Module.HEAPU32.buffer, boardPtr, 8 * 8);
    boardHeap.fill(0);

    // Allocate memory for output parameters
    const maxOutputStrLen = 10; // Adjust as necessary
    const turnPtr = Module._malloc(maxOutputStrLen);
    const castlingPtr = Module._malloc(maxOutputStrLen);
    const enPassantPtr = Module._malloc(maxOutputStrLen);
    const halfMoveClockPtr = Module._malloc(maxOutputStrLen);
    const fullMoveNumberPtr = Module._malloc(maxOutputStrLen);

    // Initialize the output memory
    Module.HEAPU8.fill(0, turnPtr, turnPtr + maxOutputStrLen);
    Module.HEAPU8.fill(0, castlingPtr, castlingPtr + maxOutputStrLen);
    Module.HEAPU8.fill(0, enPassantPtr, enPassantPtr + maxOutputStrLen);
    Module.HEAPU8.fill(0, halfMoveClockPtr, halfMoveClockPtr + maxOutputStrLen);
    Module.HEAPU8.fill(0, fullMoveNumberPtr, fullMoveNumberPtr + maxOutputStrLen);

    const r1 = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
        prompt: '> '
    });

    console.log('Hemi Hemi Chess Console');
    r1.prompt();

    r1.on('line', (line) => {
        const input = line.trim();

        // Handle commands
        if (input.startsWith('position fen ')) {
            const fenString = input.substring('position fen '.length).trim().replace(/"/g, '');
            setBoardFromFen(boardPtr, fenString, turnPtr, castlingPtr, enPassantPtr, halfMoveClockPtr, fullMoveNumberPtr);
        } else if (input.startsWith('go perft ')) {
            const depthStr = input.substring('go perft '.length).trim();
            const depth = parseInt(depthStr, 10);
            if (!isNaN(depth) && depth > 0) {
                perft(depth);
            } else {
                console.log('Invalid depth for perft.');
            }
        } else if (input === 'd') {
            printBoard();
        } else if (input === 'exit' || input === 'quit') {
            rl.close();
        } else {
            console.log('Unknown command:', input);
        }

        r1.prompt();
    }).on('close', () => {
        freeAll();
    })

    function perft (depth) {
        console.log("searching to depth", depth);
    }

    function printBoard() {
        const turn = Module.UTF8ToString(turnPtr);
        const castling = Module.UTF8ToString(castlingPtr);
        const enPassant = Module.UTF8ToString(enPassantPtr);
        const halfMoveClock = Module.UTF8ToString(halfMoveClockPtr);
        const fullMoveNumber = Module.UTF8ToString(fullMoveNumberPtr);

        console.log('Turn:', turn);
        console.log('Castling:', castling);
        console.log('En Passant:', enPassant);
        console.log('Half Move Clock:', halfMoveClock);
        console.log('Full Move Number:', fullMoveNumber);

        const columns = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'];
    
        let header = '    ';
        for (const col of columns) {
            header += ` ${col}   `;
        }
        console.log(header);
    
        console.log('  +' + '-'.repeat(39) + '+');
    
        for (let row = 0; row < 8; row++) {
            const displayRowNumber = 8 - row; 
            let rowStr = `${displayRowNumber} |`;  
            for (let col = 0; col < 8; col++) {
                const index = (row * 8) + col;
                const strPtr = boardHeap[index];
                let piece = '  ';  
                if (strPtr !== 0) {
                    piece = Module.UTF8ToString(strPtr);
                }
                rowStr += ` ${piece} |`;
            }
            console.log(rowStr);
    
            console.log('  +' + '-'.repeat(39) + '+');
        }
    }
    
    function freeAll() {
        Module._free(turnPtr);
        Module._free(castlingPtr);
        Module._free(enPassantPtr);
        Module._free(halfMoveClockPtr);
        Module._free(fullMoveNumberPtr);

        for (let i = 0; i < 8 * 8; i++) {
            const strPtr = boardHeap[i];
            if (strPtr !== 0) {
                Module._free(strPtr);
            }
        }
    
        // Free the board pointer
        Module._free(boardPtr);

        console.log("closing HHCC");
    }

})();