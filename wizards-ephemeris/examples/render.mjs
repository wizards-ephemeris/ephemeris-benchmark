import { readFileSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const root = dirname(fileURLToPath(import.meta.url))
const catalog = JSON.parse(readFileSync(join(root, 'catalog.json'), 'utf8'))

const intro = [
  'These programs are the calls a chart makes.',
  'The date, unless a story says otherwise, is 1 January 2000 at 12:00 UT.',
  'Houses use 50.22° N and 18.67° E.',
  'On the documentation site the language control switches every sample together, and the choice is remembered.',
  ''
].join('\n')

function region(path) {
  const text = readFileSync(path, 'utf8')
  const begin = text.indexOf('// doc:begin\n')
  const end = text.indexOf('\n  // doc:end')
  if (begin < 0 || end < 0 || end < begin) {
    throw new Error(`missing doc markers in ${path}`)
  }
  const body = text.slice(begin + '// doc:begin\n'.length, end)
  return body.split('\n').map((line) => line.startsWith('  ') ? line.slice(2) : line).join('\n')
}

function page() {
  const parts = ['# Examples', '', intro]
  let group = ''
  for (const item of catalog) {
    if (item.group !== group) {
      group = item.group
      parts.push(`## ${group}`, '')
      if (item.groupIntro) parts.push(item.groupIntro, '')
    }
    const cpp = region(join(root, 'cpp', `${item.id}.cpp`))
    const ts = region(join(root, 'ts', `${item.id}.ts`))
    parts.push(
      `### ${item.title}`,
      '',
      item.summary,
      '',
      `::: example ${item.title}`,
      '```cpp',
      '#include "eph/eph.hpp"',
      '',
      cpp,
      '```',
      '',
      '```ts',
      "import { Ephemeris } from 'wizards-ephemeris'",
      '',
      ts,
      '```',
      ':::',
      ''
    )
  }
  return parts.join('\n')
}

const targets = [
  join(root, '../docs/examples.md'),
  join(root, '../../wizards-ephemeris-site/docs/examples.md')
]

const rendered = page()
const check = process.argv.includes('--check')
if (check) {
  let drifted = false
  for (const target of targets) {
    const current = readFileSync(target, 'utf8')
    if (current !== rendered) {
      process.stderr.write(`out of date: ${target}\n`)
      drifted = true
    }
  }
  process.exit(drifted ? 1 : 0)
}

for (const target of targets) writeFileSync(target, rendered)
