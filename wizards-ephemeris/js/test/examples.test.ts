import assert from 'node:assert/strict'
import { spawnSync } from 'node:child_process'
import { test } from 'node:test'
import { fileURLToPath } from 'node:url'
import { runExampleTests } from './exampleChecks.ts'

test('documented examples match their stories', async () => {
  assert.deepEqual(await runExampleTests(), [])
})

test('examples page matches the sources', () => {
  const script = fileURLToPath(new URL('../../examples/render.mjs', import.meta.url))
  const result = spawnSync(process.execPath, [script, '--check'], { encoding: 'utf8' })
  assert.equal(result.status, 0, result.stdout + result.stderr)
})
