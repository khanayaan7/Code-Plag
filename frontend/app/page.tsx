"use client";

import { FormEvent, useState } from "react";

import CodeInput from "@/components/CodeInput";
import ResultView from "@/components/ResultView";
import { compareCode, type CompareResult } from "@/lib/api";

export default function Home() {
  const [code1, setCode1] = useState("");
  const [code2, setCode2] = useState("");
  const [result, setResult] = useState<CompareResult | null>(null);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  const canCompare = code1.trim().length > 0 && code2.trim().length > 0 && !loading;

  async function handleCompare(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();
    if (!canCompare) return;

    setLoading(true);
    setError("");
    setResult(null);

    try {
      setResult(await compareCode(code1, code2));
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : "Comparison failed. Please try again.");
    } finally {
      setLoading(false);
    }
  }

  return (
    <main className="relative min-h-screen overflow-hidden px-4 py-10 sm:px-6 lg:px-8">
      <div aria-hidden="true" className="pointer-events-none absolute inset-0 bg-[radial-gradient(circle_at_top_left,rgba(8,145,178,0.16),transparent_32%),radial-gradient(circle_at_85%_20%,rgba(245,158,11,0.08),transparent_25%)]" />
      <div className="relative mx-auto max-w-7xl">
        <header className="mb-9 max-w-3xl">
          <div className="mb-4 inline-flex items-center gap-2 rounded-full border border-cyan-400/20 bg-cyan-400/5 px-3 py-1 text-xs font-semibold uppercase tracking-[0.18em] text-cyan-300">
            <span className="h-1.5 w-1.5 rounded-full bg-cyan-400" />
            Rolling hash + winnowing
          </div>
          <h1 className="text-4xl font-bold tracking-tight text-white sm:text-5xl">
            Find copied logic,
            <span className="text-cyan-400"> beyond renamed variables.</span>
          </h1>
          <p className="mt-4 max-w-2xl text-base leading-7 text-slate-400 sm:text-lg">
            Compare two source files locally. Comments, formatting, identifiers, and literals are normalized before fingerprinting.
          </p>
        </header>

        <form onSubmit={handleCompare}>
          <div className="grid gap-5 lg:grid-cols-2">
            <CodeInput label="File A" code={code1} onChange={setCode1} />
            <CodeInput label="File B" code={code2} onChange={setCode2} />
          </div>

          <div className="mt-6 flex flex-col items-center gap-3">
            <button
              type="submit"
              disabled={!canCompare}
              className="min-w-48 rounded-xl bg-cyan-400 px-7 py-3.5 font-bold text-slate-950 shadow-lg shadow-cyan-500/20 transition hover:bg-cyan-300 focus:outline-none focus:ring-2 focus:ring-cyan-300 focus:ring-offset-2 focus:ring-offset-slate-950 disabled:cursor-not-allowed disabled:bg-slate-700 disabled:text-slate-400 disabled:shadow-none"
            >
              {loading ? "Comparing…" : "Compare code"}
            </button>
            <p className="text-xs text-slate-500">Your source code is not stored.</p>
          </div>
        </form>

        {error && (
          <div role="alert" className="mx-auto mt-8 max-w-2xl rounded-xl border border-red-800/70 bg-red-950/50 px-5 py-4 text-center text-sm text-red-200">
            {error}
          </div>
        )}

        {result && <ResultView result={result} code1={code1} code2={code2} />}
      </div>
    </main>
  );
}
