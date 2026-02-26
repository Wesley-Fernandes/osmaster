const native = require('./build/Release/osmaster_native.node');
const path = require('path');

const pbfPath = path.resolve('D:/Youbingo/Osmaster/PBF/sul-260225.osm.pbf');

console.log('--- OSM Load Test ---');
console.log('File:', pbfPath);

try {
    const start = Date.now();
    const result = native.loadOsm(pbfPath);
    const end = Date.now();
    
    console.log('Result:', result);
    console.log('Time taken:', (end - start) / 1000, 'seconds');
} catch (err) {
    console.error('Error loading OSM:', err);
}
