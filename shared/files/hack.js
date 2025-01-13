const fs = require('fs');
const path = require('path');

const parentDir = path.resolve(__dirname, '/');
const filePath = path.join(parentDir, 'a.txt');

fs.writeFile(filePath, '', (err) => {
  if (err) {
    console.error('Error creating file:', err);
    process.exit(1);
  }
  console.log('File created successfully in parent directory');
});
