# OSM Native Routing Engine for Node.js

## 1. Objetivo

Desenvolver uma biblioteca nativa em C++ com bindings para Node.js (TypeScript-first) capaz de:

- Carregar dados `.osm.pbf`
- Construir grafo viário otimizado
- Executar roteamento entre dois pontos geográficos
- Retornar geometria, distância e duração
- Operar 100% localmente, sem APIs externas

API desejada:

```ts
const osm = new Osm({ path: "./data/sul-260225.osm.pbf" });

const way = osm.way({
  start: { lat: -23.55052, lon: -46.6333 },
  end: { lat: -23.551, lon: -46.6333 },
});

console.log(way);
```

---

# 2. Stack Técnica

## Core

- C++20
- libosmium (parse OSM)
- Node-API (N-API)
- node-addon-api (wrapper C++)
- CMake

## Opcional (performance)

- mmap para loading binário
- SIMD otimizações
- Multi-threading para preprocessamento

---

# 3. Arquitetura Geral

```
/core
  graph.hpp
  graph.cpp
  router.hpp
  router.cpp
  spatial_index.hpp
  spatial_index.cpp
  osm_loader.hpp
  osm_loader.cpp

/bindings
  addon.cpp
  osm_wrapper.cpp

/include
  public_api.hpp

/types
  index.d.ts

CMakeLists.txt
package.json
```

---

# 4. Pipeline de Processamento

## 4.1 Fase 1 – Parsing

Entrada:

```
.osm.pbf
```

Extrair apenas:

- nodes
- ways com tag `highway`

Ignorar:

- buildings
- landuse
- relations irrelevantes

---

## 4.2 Fase 2 – Construção do Grafo

Modelo interno:

```
Node:
  id
  lat
  lon

Edge:
  from_index
  to_index
  weight
```

Representação:

- Adjacency List compacta
- Vetores contínuos em memória
- IDs mapeados para índices sequenciais

Peso padrão:

- Distância Haversine

Extensível para:

- Tempo estimado
- Tipo de via
- Penalidades

---

## 4.3 Fase 3 – Índice Espacial

Implementar KD-Tree customizada:

Função obrigatória:

```
nearestNode(lat, lon)
```

Requisitos:

- Busca em O(log n)
- Estrutura imutável após build
- Armazenar índices, não cópias

---

## 4.4 Fase 4 – Algoritmo de Rota

Implementar A\*.

Heurística:

- Haversine até destino

Estrutura necessária:

- Min-heap (priority queue)
- Vetor de distâncias
- Vetor de predecessores

Complexidade esperada:
O(E log V)

---

# 5. Interface C++ Interna

```cpp
class Router {
public:
  Router(const std::string& osmPath);
  RouteResult route(Coordinate start, Coordinate end);
};
```

RouteResult:

```
distance
duration
geometry (vector<Coordinate>)
```

---

# 6. Binding Node.js

Classe exposta:

```ts
export class Osm {
  constructor(opts: { path: string });
  way(req: WayRequest): WayResponse;
}
```

WayRequest:

```ts
{
  start: {
    lat: number;
    lon: number;
  }
  end: {
    lat: number;
    lon: number;
  }
}
```

WayResponse:

```ts
{
  distance: number;
  duration: number;
  geometry: {
    lat: number;
    lon: number;
  }
  [];
}
```

---

# 7. Inicialização

O construtor deve:

1. Parsear OSM
2. Construir grafo
3. Construir KD-tree
4. Manter tudo em memória

Carregamento síncrono inicialmente.

Opcional:

- Versão assíncrona com Worker Thread

---

# 8. Performance Esperada

Região média (estado ou grande cidade):

- Load inicial: 5–30s
- Memória: 200MB–1GB
- Snap: < 1ms
- Rota média: 1–10ms

---

# 9. Fase 2 (Evolução)

Após versão funcional:

- Serialização binária do grafo
- Loader direto de `.graph` pré-processado
- Contraction Hierarchies
- Multi-threaded preprocessing
- Perfis de veículo (car, bike, walk)
- WASM build

---

# 10. Testes

Criar:

- Teste unitário de Haversine
- Teste de KD-tree
- Teste de A\*
- Teste de rota real curta
- Benchmark suite

---

# 11. Critérios de Aceitação

A lib é considerada pronta quando:

- Consegue carregar `.osm.pbf`
- Retorna rota válida entre dois pontos
- Não usa dependências externas de roteamento
- Executa rota em < 20ms para cidade média
- Possui tipagem TypeScript completa
- Compila em Linux e Windows

---

# 12. Não Objetivos (Inicial)

- Trânsito em tempo real
- Turn-by-turn instructions
- Renderização de mapa
- Otimização multi-destino
- Clustering

---

# 13. Entregáveis

- Biblioteca compilável
- Binário `.node`
- Tipos TypeScript
- Script de build
- Benchmark
- Exemplo de uso

---

# 14. Roadmap

Fase 1
✔ Parser
✔ Grafo
✔ KD-tree
✔ A\*
✔ Binding Node

Fase 2
✔ Serialização binária
✔ CH
✔ Perfis de rota

Fase 3
✔ WASM
✔ Multi-thread
