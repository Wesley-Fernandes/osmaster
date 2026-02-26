# OSM Native Routing Engine - TODO List

## Fase 1: MVP (Mínimo Produto Viável)

- [x] Configuração do Ambiente (C++, Node-API, CMake)
- [x] **Fase 1: Parsing** - Extração de dados `.osm.pbf` (libosmium)
- [x] **Fase 2: Grafo** - Construção do grafo em memória (Adjacency List)
- [/] **Fase 3: Índice Espacial** - Implementação da KD-Tree (nearestNode)
- [ ] **Fase 4: Algoritmo de Rota** - Implementação do A\* com Haversine
- [ ] **Bindings Node.js** - Wrapper C++ para uso no TypeScript
- [ ] Testes Unitários e Validação de Rotas

## Fase 2: Otimização e Performance

- [ ] **Fase 2: Contraction Hierarchies (CH)**
  - [ ] Node Ranking (Edge Difference, etc.)
  - [ ] Node Contraction (Shortcut generation)
  - [ ] Bidirectional DIjkstra Query
  - [ ] Serialização binária do grafo contraído
- [ ] Multi-level graph concepts (CRP/Overlay) para atualizações rápidas
- [ ] Perfis de Veículo (Carro, Bicicleta, Pedestre)
- [ ] Multi-threaded Preprocessing

## Fase 3: Portabilidade e Escala

- [ ] Compilação para WebAssembly (WASM)
- [ ] Otimizações SIMD
- [ ] Suporte a restrições de manobra (U-turns, etc.)

---

_Gerado automaticamente com base no `plan.md`._
