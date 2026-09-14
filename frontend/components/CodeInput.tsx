"use client";

import { ChangeEvent, useRef, useState } from "react";

interface CodeInputProps {
  label: string;
  code: string;
  onChange: (code: string) => void;
}

export default function CodeInput({ label, code, onChange }: CodeInputProps) {
  const fileInput = useRef<HTMLInputElement>(null);
  const [fileName, setFileName] = useState("");
  const [fileError, setFileError] = useState("");

  async function handleFile(event: ChangeEvent<HTMLInputElement>) {
    const file = event.target.files?.[0];
    if (!file) return;

    try {
      onChange(await file.text());
      setFileName(file.name);
      setFileError("");
    } catch {
      setFileError("This file could not be read.");
    }
  }

  return (
    <section className="overflow-hidden rounded-2xl border border-slate-700/80 bg-slate-900/70 shadow-2xl shadow-black/20">
      <div className="flex min-h-16 items-center justify-between gap-4 border-b border-slate-700/80 px-5 py-3">
        <div>
          <h2 className="font-semibold text-slate-100">{label}</h2>
          <p className="mt-0.5 max-w-48 truncate text-xs text-slate-400">
            {fileName || "Paste code or choose a file"}
          </p>
        </div>
        <div>
          <input
            ref={fileInput}
            className="sr-only"
            type="file"
            accept=".c,.cc,.cpp,.cxx,.h,.hpp,.java,.js,.jsx,.ts,.tsx,.py,.txt"
            onChange={handleFile}
            aria-label={`Upload ${label}`}
          />
          <button
            type="button"
            onClick={() => fileInput.current?.click()}
            className="rounded-lg border border-slate-600 bg-slate-800 px-3 py-2 text-sm font-medium text-slate-200 transition hover:border-cyan-400/70 hover:text-cyan-300 focus:outline-none focus:ring-2 focus:ring-cyan-400"
          >
            Upload file
          </button>
        </div>
      </div>
      <label className="sr-only" htmlFor={`${label}-code`}>
        {label} code
      </label>
      <textarea
        id={`${label}-code`}
        value={code}
        onChange={(event) => onChange(event.target.value)}
        spellCheck={false}
        placeholder="Paste source code here…"
        className="h-80 w-full resize-y bg-transparent p-5 font-mono text-sm leading-6 text-slate-200 outline-none placeholder:text-slate-600 focus:bg-slate-900/60"
      />
      {fileError && (
        <p role="alert" className="border-t border-red-900/60 bg-red-950/40 px-5 py-2 text-sm text-red-300">
          {fileError}
        </p>
      )}
    </section>
  );
}
