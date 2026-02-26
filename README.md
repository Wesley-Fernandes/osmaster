# OSMaster 🚀

**OSMaster** é um motor de roteamento nativo de ultra-performance para Node.js e TypeScript. Ele processa dados do OpenStreetMap (.osm.pbf) para fornecer cálculos de rota e busca de coordenadas em milissegundos.

## 🌟 Destaques

- **Alta Performance**: Escrito em C++20 com algoritmos otimizados.
- **Busca Espacial**: KD-Tree integrada para localização instantânea de nós.
- **Roteamento Inteligente**: Implementação eficiente de A\* com suporte a ruas de mão única (`oneway`).
- **Nativo**: Utiliza N-API para máxima integração com o runtime do Node.js.

## 📦 Instalação

```bash
npm install
npm run build     # Compila o motor nativo (C++)
npm run build:ts  # Compila o wrapper TypeScript
```

## 🛠️ Guia de Uso (TypeScript)

```typescript
import { OSMaster } from "./lib/index";

const osm = new OSMaster();

// 1. Carregar o mapa (Build inicial do grafo)
// Isso processa o PBF e constrói o índice espacial em memória.
osm.load("caminho/para/brasil-sul.osm.pbf");

// 2. Encontrar o ponto mais próximo no mapa
const start = { lat: -27.5954, lon: -48.548 };
const nearest = osm.findNearestNode(start.lat, start.lon);
console.log(`Nó encontrado: ${nearest.id}`);

// 3. Calcular uma rota entre dois pontos
const end = { lat: -25.4297, lon: -49.2711 };
const route = osm.calculateRoute(start, end);

if (route.found) {
  console.log(`Distância: ${(route.distance / 1000).toFixed(2)} km`);
  console.log(`Geometria: ${route.path.length} pontos`);
}
```

## 📊 Benchmarks (Região Sul do Brasil)

- **Parsing e Indexação**: ~35-40 segundos.
- **Busca de Ponto (KD-Tree)**: < 1ms.
- **Cálculo de Rota (300km)**: ~1.9 segundos.

## 🏗️ Estrutura do Projeto

- `/core`: Código C++ (A\*, Graph, KD-Tree).
- `/src`: Wrapper TypeScript.
- `/lib`: Binários e tipos compilados.
- `/bindings`: Interface N-API (Glue code).

## 📄 Licença

Distribuído sob a licença MIT.
