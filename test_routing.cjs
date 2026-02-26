const native = require('./build/Release/osmaster_native.node');
const path = require('path');

const pbfPath = path.resolve('D:/Youbingo/Osmaster/PBF/sul-260225.osm.pbf');

console.log('--- Routing Engine Test (A*) ---');

try {
    console.log('Loading Graph and Index...');
    native.buildGraph(pbfPath);

    // From Florianópolis to Curitiba (approx)
    // Floripa: -27.5954, -48.5480
    // Curitiba: -25.4297, -49.2711
    
    console.log('\nCalculating route: Florianópolis -> Curitiba');
    const start = Date.now();
    const route = native.findRoute(-27.5954, -48.5480, -25.4297, -49.2711);
    const end = Date.now();

    console.log('Route Result:', {
        found: route.found,
        distanceMeters: route.distance.toFixed(2),
        distanceKm: (route.distance / 1000).toFixed(2),
        nodeCount: route.path ? route.path.length : 0,
        timeMs: end - start
    });

    if (route.found && route.path.length > 0) {
        console.log('\nSample Path Points:');
        route.path.slice(0, 5).forEach((p, i) => console.log(`  ${i}: ${p.lat}, ${p.lon}`));
        console.log('  ...');
        route.path.slice(-5).forEach((p, i) => console.log(`  ${route.path.length - 5 + i}: ${p.lat}, ${p.lon}`));
        
        console.log('\nSUCCESS: Route found and validated.');
    } else {
        console.error('\nERROR: Route not found!');
    }

} catch (err) {
    console.error('Error during routing test:', err);
}
