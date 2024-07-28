const express = require('express');
const http = require('http');
const socketIO = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketIO(server);

app.use(express.static('public'));
app.use(express.json());

// Variable to store the pending command for NodeMCU
let pendingCommand = "";

app.post('/set-threshold', (req, res) => {
  let thresholdValue = req.body.threshold;
  // Store the command for NodeMCU
  pendingCommand = 'threshold:' + thresholdValue;
  res.json({ message: 'Threshold value set successfully!' });
});

app.get('/switch-1', (req, res) => {
  // Store the command for NodeMCU
  pendingCommand = 'LED: 1';
  res.send('Switched ON red LED');
});

app.get('/switch-2', (req, res) => {
  // Store the command for NodeMCU
  pendingCommand = 'LED: 2';
  res.send('Switched OFF red LED');
});

app.get('/switch-3', (req, res) => {
  // Store the command for NodeMCU
  pendingCommand = 'LED: 3';
  res.send('Switched ON green LED');
});

app.get('/switch-4', (req, res) => {
  // Store the command for NodeMCU
  pendingCommand = 'LED: 4';
  res.send('Switched OFF green LED');
});

// New endpoint for NodeMCU to retrieve the command
app.get('/get-command', (req, res) => {
  res.json({ command: pendingCommand });
  pendingCommand = "";  // Reset the command after it's retrieved
});

app.post('/data-from-nodemcu', (req, res) => {
  console.log("Received data:", req.body); // Log the raw data received
  try {
    const jsonData = req.body;
    io.emit('data', jsonData);
    res.json({ message: 'Data received successfully!' });
  } catch (error) {
    console.error('Error parsing data from NodeMCU:', error);
    res.status(400).json({ message: 'Invalid data format' });
  }
});

io.on('connection', (socket) => {
  console.log('A user connected');
  socket.on('disconnect', () => {
    console.log('A user disconnected');
  });
});

server.listen(80, () => {
  console.log('Server is running on 8080');
});
