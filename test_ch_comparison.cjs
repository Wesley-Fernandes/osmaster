const native = require('./build/Release/osmaster_native.node');
const path = require('path');

const pbfPath = path.resolve('D:/Youbingo/Osmaster/PBF/sul-260225.osm.pbf');

async function runTest() {
    console.log('--- Routing Performance Comparison: A* vs CH ---');

    try {

        native.buildGraph(pbfPath);
        
        console.log('1. Loading Graph and Index...');
        native.buildGraph(pbfPath);

        // Coordinates for Rio Grande do Sul: Porto Alegre -> Pelotas
        const startLat = -30.0346, startLon = -51.2177;
        const endLat = -31.7654, endLon = -52.3376;

        console.log('\n2. Testing A* (Baseline)...');
        const startAstar = Date.now();
        const routeAstar = native.findRoute(startLat, startLon, endLat, endLon);
        const endAstar = Date.now();
        const timeAstar = endAstar - startAstar;

        console.log('A* Results:', {
            found: routeAstar.found,
            distanceKm: (routeAstar.distance / 1000).toFixed(2),
            timeMs: timeAstar
        });
        /*
        console.log('\n3. Running CH Preprocessing (One-time)...');
        console.time('CH_Preprocessing');
        native.runCHPreprocessing();
        console.timeEnd('CH_Preprocessing');

        console.log('\n4. Testing CH (Optimized)...');
        const startCH = Date.now();
        const routeCH = native.findRouteCH(startLat, startLon, endLat, endLon);
        const endCH = Date.now();
        const timeCH = endCH - startCH;

        console.log('CH Results:', {
            found: routeCH.found,
            distanceKm: (routeCH.distance / 1000).toFixed(2),
            timeMs: timeCH
        });

        console.log('\n--- Final Comparison ---');
        console.log(`Speedup: ${(timeAstar / timeCH).toFixed(1)}x faster`);
        const distDiff = Math.abs(routeAstar.distance - routeCH.distance);
        console.log(`Distance Difference: ${distDiff.toFixed(4)} meters`);

        if (distDiff < 0.1) {
            console.log('SUCCESS: Accuracy verified (Same distance as A*).');
        } else {
            console.warn('WARNING: Distance mismatch! Check implementation.');
        }
            */

    } catch (err) {
        console.error('Error during comparison test:', err);
    }
}

runTest();
