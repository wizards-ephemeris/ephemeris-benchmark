import { expect, test } from '@playwright/test'

test('wasm cases in Chromium', async ({ page }) => {
  await page.goto('/test/page.html')
  const body = page.locator('body')
  await expect(body).toHaveAttribute('data-ready', /.*/, { timeout: 60_000 })
  const text = await body.innerText()
  await expect(body, text).toHaveAttribute('data-ready', '1')
})
