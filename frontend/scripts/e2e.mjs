import { existsSync } from "node:fs";

import { chromium } from "playwright-core";

const browserCandidates = [
  process.env.PLAYWRIGHT_BROWSER_PATH,
  "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
  "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
  "/usr/bin/google-chrome",
  "/usr/bin/chromium",
].filter(Boolean);

const executablePath = browserCandidates.find((candidate) => existsSync(candidate));
if (!executablePath) {
  throw new Error(
    "No Chrome-compatible browser found. Set PLAYWRIGHT_BROWSER_PATH to run this check.",
  );
}

const browser = await chromium.launch({ executablePath, headless: true });
const page = await browser.newPage();
const pageErrors = [];
const baseUrl = (process.env.E2E_BASE_URL ?? "http://localhost:3000").replace(
  /\/$/,
  "",
);
page.on("pageerror", (error) => pageErrors.push(error.message));

try {
  await page.goto(baseUrl, { waitUntil: "networkidle" });

  const compareButton = page.getByRole("button", { name: "Compare code" });
  if (!(await compareButton.isDisabled())) {
    throw new Error("Compare button should start disabled.");
  }

  const firstInput = page.getByLabel("File A code");
  const secondInput = page.getByLabel("File B code");
  const uploadedCode = `int sum(int first, int second) {
  int result = first + second;
  return result;
}`;
  await page.getByLabel("Upload File A").setInputFiles({
    name: "sum.cpp",
    mimeType: "text/plain",
    buffer: Buffer.from(uploadedCode),
  });
  if ((await firstInput.inputValue()) !== uploadedCode) {
    throw new Error("Uploading a file did not populate the File A textarea.");
  }
  await secondInput.fill(`int combine(int x, int y) {
  int answer = x + y;
  return answer;
}`);

  if (await compareButton.isDisabled()) {
    throw new Error("Compare button did not enable after both inputs were filled.");
  }
  await compareButton.click();
  await page.getByTestId("similarity").filter({ hasText: "100%" }).waitFor();

  const highlightedLineCount = await page.locator('[data-matched="true"]').count();
  if (highlightedLineCount < 2) {
    throw new Error("Expected matching lines to be highlighted in both files.");
  }

  await firstInput.fill(`int calculate(int a, int b) {
  int product = a * b;
  return product + 42;
}`);
  await secondInput.fill(`void inspect(bool ready) {
  while (ready) {
    if (ready) break;
    else continue;
  }
}`);
  await compareButton.click();
  await page.getByTestId("similarity").waitFor();
  const unrelatedSimilarity = Number(
    (await page.getByTestId("similarity").textContent())?.replace("%", ""),
  );
  if (!Number.isFinite(unrelatedSimilarity) || unrelatedSimilarity > 20) {
    throw new Error(`Expected unrelated similarity <= 20%, got ${unrelatedSimilarity}%.`);
  }

  if (pageErrors.length > 0) {
    throw new Error(`Browser page errors: ${pageErrors.join("; ")}`);
  }

  console.log(
    `E2E passed: upload populated the editor, renamed code = 100%, unrelated code = ${unrelatedSimilarity}%, ${highlightedLineCount} highlighted lines.`,
  );
} finally {
  await browser.close();
}
