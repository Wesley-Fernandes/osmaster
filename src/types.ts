export interface LatLon {
    lat: number;
    lon: number;
}

export interface NodeInfo extends LatLon {
    id: number;
    index: number;
}

export interface RouteResult {
    found: boolean;
    distance: number; // in meters
    path: LatLon[];
}

export interface GraphStats {
    nodes: number;
    edges: number;
}

export interface NativeModule {
    buildGraph(path: string): GraphStats;
    findNearestNode(lat: number, lon: number): NodeInfo;
    findRoute(sLat: number, sLon: number, eLat: number, eLon: number): RouteResult;
}
