# codeplag

`codeplag` compares two source-code submissions and finds structurally similar
regions even when identifiers, literals, comments, and formatting have changed.
It is a deliberately small implementation of the core fingerprinting approach
used by systems such as MOSS and JPlag.

The project has two components:

- `backend/`: a C++17 HTTP service using `cpp-httplib` and `nlohmann/json`.
- `frontend/`: a Next.js App Router interface written in TypeScript and styled
  with Tailwind CSS.

There is no database, authentication, or source-code storage.

## Live demo

The production application is deployed at
**[code-plag-ten.vercel.app](https://code-plag-ten.vercel.app)**.

## How the detector works

1. **Normalize tokens.** Comments and whitespace are removed, literals become
   `NUM` or `STR`, and non-keyword identifiers become `ID`. Each token retains
   its original source line.
2. **Create rolling hashes.** Every five-token k-gram is hashed with a
   polynomial rolling hash using base 131 and natural `uint64_t` wraparound.
3. **Winnow fingerprints.** A four-hash window selects its minimum hash, using
   the rightmost value when minima tie. Repeated selection of the same position
   in overlapping windows is suppressed.
4. **Calculate containment.** The number of shared unique fingerprint hashes is
   divided by the smaller unique fingerprint set. Containment is preferable to
   Jaccard here because a copied short snippet should remain highly similar even
   when it is embedded in a much longer submission.
5. **Map matches to lines.** Fingerprints shared by the two files are converted
   back to source-line ranges, then overlapping and adjacent ranges are merged.

The default parameters are `k = 5` tokens and `w = 4` hashes.

## Prerequisites

- CMake 3.24 or newer
- A C++17 compiler (GCC, Clang, or recent MSVC)
- Node.js 20.9 or newer and npm
- Docker Desktop or Docker Engine, only for the container workflow

CMake downloads `cpp-httplib`, `nlohmann/json`, and Catch2 with
`FetchContent` during the first configuration.

## Run the backend locally

From the repository root:

```bash
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run it on Linux or macOS:

```bash
./build/codeplag_server
```

With a single-config Windows generator, the executable is normally:

```powershell
.\build\codeplag_server.exe
```

With Visual Studio, add the selected configuration directory, such as
`.\build\Release\codeplag_server.exe`.

The service reads these optional environment variables:

| Variable | Default | Purpose |
| --- | --- | --- |
| `PORT` | `8080` | Backend listening port |
| `FRONTEND_ORIGIN` | `http://localhost:3000` | Allowed browser origin for CORS |

Check the running service:

```bash
curl http://localhost:8080/health
curl -X POST http://localhost:8080/compare \
  -H "Content-Type: application/json" \
  -d '{"code1":"int sum(int a,int b){return a+b;}","code2":"int add(int x,int y){return x+y;}"}'
```

## Run backend tests

After configuring and building the backend:

```bash
cd backend
ctest --test-dir build --output-on-failure
```

The Catch2 suite covers tokenization, line tracking, rolling hashes, winnowing
tie-breaking, range merging, empty and short input, unrelated files, renamed
identifiers, reformatted code, repetitive code, and identical files.

## Run the frontend

Create the optional local environment file and install dependencies:

```bash
cd frontend
cp .env.local.example .env.local
npm install
npm run dev
```

On PowerShell, use `Copy-Item .env.local.example .env.local` instead of `cp` if
the alias is unavailable. Open <http://localhost:3000>, paste or upload two
files, and select **Compare code**. The backend must be running at the URL in
`NEXT_PUBLIC_API_URL`.

Build and test the frontend:

```bash
npm test
npm run build
```

An additional real-browser E2E check is available while both development
servers are running:

```bash
npm run test:e2e
```

It uses an installed Chrome-compatible browser. Set `PLAYWRIGHT_BROWSER_PATH`
if Chrome or Edge is installed at a non-standard location.

To run the same check against the deployed application, set `E2E_BASE_URL`:

```powershell
$env:E2E_BASE_URL = "https://code-plag-ten.vercel.app"
npm run test:e2e
```

## Deploy to Vercel

The root `vercel.json` deploys the Next.js frontend and C++ backend as separate
container services, then routes `/compare` and `/health` to the backend. With
the Vercel CLI installed and authenticated, deploy from the repository root:

```bash
npx vercel@latest deploy --prod
```

Vercel builds each service from its `Dockerfile.vercel`. The frontend uses a
relative API URL in production, so no public backend URL or production
`NEXT_PUBLIC_API_URL` is required.

## Run the backend with Docker

Make sure Docker Desktop is open and reports that the engine is running.

### Build the image

From the repository root, build the backend image once:

```powershell
docker build -t codeplag-backend:local ./backend
```

### Run it from Docker Desktop

1. Open the **Images** view in Docker Desktop.
2. Find `codeplag-backend` with the `local` tag and select **Run**.
3. Expand **Optional settings**.
4. Set the container name to `codeplag-backend`.
5. Map host port `8080` to container port `8080`.
6. Add the environment variable `FRONTEND_ORIGIN` with the value
   `http://localhost:3000`.
7. Select **Run**.

Open <http://localhost:8080/health>. A working container returns:

```json
{"status":"ok"}
```

Start the frontend separately with `npm run dev` from `frontend/`, then open
<http://localhost:3000>. Use the **Containers** view in Docker Desktop to view
logs, stop, restart, or remove the backend container.

### Run it from a terminal

The equivalent terminal commands are:

```bash
docker build -t codeplag-backend:local ./backend
docker run --rm --name codeplag-backend -p 127.0.0.1:8080:8080 \
  -e FRONTEND_ORIGIN=http://localhost:3000 \
  codeplag-backend:local
```

The `127.0.0.1` binding keeps port `8080` accessible only from the local
computer. Do not run this container while the locally compiled backend is using
the same port.

The multi-stage build compiles in a full Debian build environment and copies
only the server binary into the slim runtime image. The runtime runs as the
unprivileged `nobody` user and includes a `/health` container health check.

## API

### `GET /health`

Returns:

```json
{"status":"ok"}
```

### `POST /compare`

Request:

```json
{
  "code1": "int sum(int a, int b) { return a + b; }",
  "code2": "int add(int x, int y) { return x + y; }"
}
```

Response:

```json
{
  "similarity": 100.0,
  "matchedLines1": [[1, 1]],
  "matchedLines2": [[1, 1]],
  "totalFingerprints1": 6,
  "totalFingerprints2": 6,
  "matchedFingerprints": 6
}
```

Missing or non-string `code1`/`code2` values and malformed JSON produce HTTP
`400` with an `{ "error": "..." }` response.

## Project layout

```text
backend/
  src/                 tokenizer, hashing, winnowing, comparison, HTTP server
  tests/               Catch2 unit and end-to-end algorithm tests
  CMakeLists.txt
  Dockerfile
frontend/
  app/                  page, layout, global Tailwind import, smoke test
  components/           code input and highlighted result views
  lib/api.ts            typed backend API wrapper
  scripts/e2e.mjs       local real-browser workflow verification
```
