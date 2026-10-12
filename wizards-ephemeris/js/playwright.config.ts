import { defineConfig } from '@playwright/test'

export default defineConfig({
  testDir: 'test',
  testMatch: 'browser.spec.ts',
  use: { baseURL: 'http://127.0.0.1:5198' },
  webServer: {
    command: 'npx vite --host 127.0.0.1 --port 5198 --strictPort',
    url: 'http://127.0.0.1:5198/test/page.html',
    reuseExistingServer: true
  }
})
