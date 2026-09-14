import type { CompareResult, LineRange } from "@/lib/api";

interface ResultViewProps {
  result: CompareResult;
  code1: string;
  code2: string;
}

function lineIsMatched(line: number, ranges: LineRange[]) {
  return ranges.some(([start, end]) => line >= start && line <= end);
}

function CodePreview({ code, ranges, label }: { code: string; ranges: LineRange[]; label: string }) {
  return (
    <div className="min-w-0 overflow-hidden rounded-xl border border-slate-700 bg-slate-950">
      <h3 className="border-b border-slate-800 px-4 py-3 text-sm font-semibold text-slate-300">
        {label}
      </h3>
      <pre className="max-h-96 overflow-auto py-3 text-sm leading-6">
        {code.split("\n").map((line, index) => {
          const lineNumber = index + 1;
          const matched = lineIsMatched(lineNumber, ranges);
          return (
            <span
              key={lineNumber}
              data-matched={matched}
              className={`grid min-w-max grid-cols-[3.5rem_1fr] px-3 ${
                matched ? "bg-amber-300/15 text-amber-50" : "text-slate-300"
              }`}
            >
              <span className={`select-none text-right ${matched ? "text-amber-400" : "text-slate-600"}`}>
                {lineNumber}
              </span>
              <code className="pl-4 pr-6">{line || " "}</code>
            </span>
          );
        })}
      </pre>
    </div>
  );
}

export default function ResultView({ result, code1, code2 }: ResultViewProps) {
  const roundedSimilarity = Math.round(result.similarity * 10) / 10;

  return (
    <section aria-live="polite" className="mt-10 rounded-2xl border border-cyan-400/20 bg-slate-900/80 p-5 shadow-2xl shadow-cyan-950/20 sm:p-7">
      <div className="mb-6 flex flex-col justify-between gap-5 sm:flex-row sm:items-end">
        <div>
          <p className="text-xs font-semibold uppercase tracking-[0.2em] text-cyan-400">
            Similarity result
          </p>
          <p data-testid="similarity" className="mt-1 text-5xl font-bold tracking-tight text-white">
            {roundedSimilarity}%
          </p>
        </div>
        <dl className="grid grid-cols-3 gap-5 text-right text-sm">
          <div>
            <dt className="text-slate-500">File A</dt>
            <dd className="mt-1 font-semibold text-slate-200">{result.totalFingerprints1}</dd>
          </div>
          <div>
            <dt className="text-slate-500">Matched</dt>
            <dd className="mt-1 font-semibold text-cyan-300">{result.matchedFingerprints}</dd>
          </div>
          <div>
            <dt className="text-slate-500">File B</dt>
            <dd className="mt-1 font-semibold text-slate-200">{result.totalFingerprints2}</dd>
          </div>
        </dl>
      </div>

      <div className="mb-4 flex items-center gap-2 text-xs text-slate-400">
        <span className="h-3 w-3 rounded-sm bg-amber-300/30" />
        Highlighted lines contain matching fingerprints
      </div>
      <div className="grid gap-5 lg:grid-cols-2">
        <CodePreview code={code1} ranges={result.matchedLines1} label="File A matches" />
        <CodePreview code={code2} ranges={result.matchedLines2} label="File B matches" />
      </div>
    </section>
  );
}
