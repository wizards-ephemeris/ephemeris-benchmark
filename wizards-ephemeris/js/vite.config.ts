import { fileURLToPath } from 'node:url'
import { defineConfig } from 'vite'

export default defineConfig({
  resolve: {
    alias: {
      '@magowie/wizards-ephemeris': fileURLToPath(new URL('./src/index.ts', import.meta.url))
    }
  },
  server: {
    host: '127.0.0.1',
    port: 5198,
    strictPort: true
  }
})
