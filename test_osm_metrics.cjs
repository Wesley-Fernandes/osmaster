const native = require('./build/Release/osmaster_native.node');
const path = require('path');

const pbfPath = path.resolve('D:/Youbingo/Osmaster/PBF/sul-260225.osm.pbf');

console.log('--- OSM Load Test with metrics ---');
console.log('File:', pbfPath);

try {
    const start = Date.now();
    const metrics = native.loadOsm(pbfPath);
    const end = Date.now();
    
    console.log('Metrics:', metrics);
    console.log('Time taken:', (end - start) / 1000, 'seconds');
    
    if (metrics.nodes > 0 && metrics.ways > 0) {
        console.log('SUCCESS: PBF loaded and parsed correctly.');
    } else {
        console.warn('WARNING: Loaded but counts are zero. Check tag filters.');
    }
} catch (err) {
    console.error('Error loading OSM:', err);
}
