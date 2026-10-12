import { copyFileSync, existsSync } from 'node:fs'

const glue = ['dist/eph.js', 'dist/eph.wasm']
const missing = glue.filter((path) => !existsSync(path))
if (missing.length > 0) {
  console.error(
    'missing ' + missing.join(' and ')
      + '. From the library directory run ./scripts/build-js.sh'
  )
  process.exit(1)
}

copyFileSync(new URL('../../LICENSE', import.meta.url), 'LICENSE')
