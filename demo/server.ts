import express from 'express';
import bodyParser from 'body-parser';
import { OSMaster } from '../src/index';
import path from 'path';

const app = express();
const port = 3000;
const osm = new OSMaster();

// Configuration
const PBF_PATH = path.resolve('D:/Youbingo/Osmaster/PBF/sul-260225.osm.pbf');

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, '.')));

console.log('OSMaster: Carregando grafo viário (isso pode levar ~40s)...');
try {
    const stats = osm.load(PBF_PATH);
    console.log(`OSMaster: Grafo pronto! (${stats.nodes} nós, ${stats.edges} edges)`);
} catch (err) {
    console.error('Erro ao carregar o mapa:', err);
    process.exit(1);
}

app.post('/api/route', (req, res) => {
    const { start, end } = req.body;
    
    if (!start || !end) {
        return res.status(400).json({ error: 'Start and End coordinates required' });
    }

    console.log(`Calculando rota: [${start.lat}, ${start.lon}] -> [${end.lat}, ${end.lon}]`);
    
    const startTime = Date.now();
    const result = osm.calculateRoute(start, end);
    const duration = Date.now() - startTime;

    res.json({
        ...result,
        timeMs: duration
    });
});

app.listen(port, () => {
    console.log(`\n🚀 OSMaster Demo rodando em: http://localhost:${port}`);
    console.log(`Abra o navegador para testar o roteamento interativo.`);
});
