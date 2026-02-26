import { NativeModule, GraphStats, NodeInfo, RouteResult, LatLon } from './types';
import path from 'path';

// Load the native binary
const native: NativeModule = require('../build/Release/osmaster_native.node');

export class OSMaster {
    private isLoaded: boolean = false;

    /**
     * Builds the routing graph from an OSM PBF file.
     * This must be called before searching or routing.
     * @param pbfPath Absolute path to the .osm.pbf file
     */
    public load(pbfPath: string): GraphStats {
        const absolutePath = path.isAbsolute(pbfPath) ? pbfPath : path.resolve(pbfPath);
        const stats = native.buildGraph(absolutePath);
        this.isLoaded = true;
        return stats;
    }

    /**
     * Finds the nearest navigable node to the given coordinates.
     */
    public findNearestNode(lat: number, lon: number): NodeInfo {
        this.checkLoaded();
        return native.findNearestNode(lat, lon);
    }

    /**
     * Calculates the shortest path between two points.
     */
    public calculateRoute(start: LatLon, end: LatLon): RouteResult {
        this.checkLoaded();
        return native.findRoute(start.lat, start.lon, end.lat, end.lon);
    }

    private checkLoaded() {
        if (!this.isLoaded) {
            throw new Error('OSMaster: No graph loaded. Call load(path) first.');
        }
    }
}

export * from './types';
