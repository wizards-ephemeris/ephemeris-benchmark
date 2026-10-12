import assert from 'node:assert/strict'
import { test } from 'node:test'
import { runCases } from './cases.ts'

test('wasm matches the native fixtures', async () => {
  const report = await runCases()
  assert.deepEqual(report.failures, [])
})
