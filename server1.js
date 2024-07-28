const express = require('express');
const http = require('http');
const socketIO = require('socket.io');
const net = require('net');

const app = express();
const server = http.createServer(app);
const io = socketIO(server);

let nodeMCUSocket = null;  // Add this line to keep reference to the NodeMCU socket

// Serve static files from the "public" directory
app.use(express.static('public'));
app.use(express.json());


// Add these lines to create endpoints for controlling the LEDs
app.post('/set-threshold', (req, res) => {
  let thresholdValue = req.body.threshold;

  // Send the threshold value to the NodeMCU
  if (nodeMCUSocket) {
      nodeMCUSocket.write('threshold:' + thresholdValue + '\n');
  }

  res.json({ message: 'Threshold value set successfully!' });
});
app.get('/switch-1', (req, res) => {
  if (nodeMCUSocket) {
    nodeMCUSocket.write('redon\n');
  }
  res.send('Switched ON red LED');
});

app.get('/switch-2', (req, res) => {
  if (nodeMCUSocket) {
    nodeMCUSocket.write('redoff\n');
  }
  res.send('Switched OFF red LED');
});

app.get('/switch-3', (req, res) => {
  if (nodeMCUSocket) {
    nodeMCUSocket.write('greenon\n');
  }
  res.send('Switched ON green LED');
});

app.get('/switch-4', (req, res) => {
  if (nodeMCUSocket) {
    nodeMCUSocket.write('greenoff\n');
  }
  res.send('Switched OFF green LED');
});

// Create a TCP server to receive data from NodeMCU
const tcpServer = net.createServer((socket) => {
  console.log('NodeMCU connected');
  nodeMCUSocket = socket;  // Add this line to save the NodeMCU socket

  socket.on('data', (data) => {
    console.log('Raw data:', data.toString());
    try {
      const jsonData = JSON.parse(data);
      io.emit('data', jsonData);
    } catch (error) {
      console.error('Error parsing data from NodeMCU:', error);
    }
  });

  socket.on('end', () => {
    console.log('NodeMCU disconnected');
    nodeMCUSocket = null;  // Add this line to clear the NodeMCU socket
  });
});

// Handler for connection and disconnection
io.on('connection', (socket) => {
  console.log('A user connected');
  socket.on('disconnect', () => {
    console.log('A user disconnected');
  });
});

// Start the server on port 3000
server.listen(8080, () => {
  console.log('Server is running on 8080');
});

// Start the TCP server on a different port from the HTTP server, for example 4000
tcpServer.listen(8090, () => {
  console.log('TCP server is listening on port 8090');
});
