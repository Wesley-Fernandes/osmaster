const native = require('./build/Release/osmaster_native.node');
const path = require('path');

const pbfPath = path.resolve('D:/Youbingo/Osmaster/PBF/sul-260225.osm.pbf');

console.log('--- Graph Construction Test ---');
console.log('File:', pbfPath);

try {
    const start = Date.now();
    const stats = native.buildGraph(pbfPath);
    const end = Date.now();
    
    console.log('Graph Stats:', stats);
    console.log('Build Time:', (end - start) / 1000, 'seconds');
    
    if (stats.nodes > 0 && stats.edges > 0) {
        console.log('SUCCESS: Graph built correctly.');
        console.log('Efficiency:', (stats.edges / stats.nodes).toFixed(2), 'edges per node.');
    } else {
        console.error('ERROR: Graph is empty!');
    }
} catch (err) {
    console.error('Error building graph:', err);
}
