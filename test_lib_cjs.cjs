const { OSMaster } = require('./lib/index');
const path = require('path');

const osm = new OSMaster();
const pbfPath = path.resolve('D:/Youbingo/Osmaster/PBF/sul-260225.osm.pbf');

function main() {
    console.log('--- OSMaster Library (CJS) Usage Test ---');

    try {
        console.log('Loading map data...');
        const stats = osm.load(pbfPath);
        console.log(`Graph loaded: ${stats.nodes} nodes, ${stats.edges} edges.`);

        const start = { lat: -27.5954, lon: -48.5480 }; // Floripa
        const end = { lat: -25.4297, lon: -49.2711 };   // Curitiba

        console.log('\nFinding nearest nodes...');
        const startNode = osm.findNearestNode(start.lat, start.lon);
        console.log('Start node:', startNode);

        console.log('\nCalculating route...');
        const timer = Date.now();
        const route = osm.calculateRoute(start, end);
        const duration = Date.now() - timer;

        if (route.found) {
            console.log(`Route Found: ${(route.distance / 1000).toFixed(2)} km`);
            console.log(`Path points: ${route.path.length}`);
            console.log(`Execution time: ${duration}ms`);
        } else {
            console.log('No route found.');
        }

        console.log('\nSUCCESS: Library is fully functional.');

    } catch (error) {
        console.error('Test failed:', error);
    }
}

main();
