export type LineRange = [number, number];

export interface CompareResult {
  similarity: number;
  matchedLines1: LineRange[];
  matchedLines2: LineRange[];
  totalFingerprints1: number;
  totalFingerprints2: number;
  matchedFingerprints: number;
}

interface ErrorResponse {
  error?: string;
}

const defaultApiUrl =
  process.env.NODE_ENV === "production" ? "" : "http://localhost:8080";
const API_URL = (process.env.NEXT_PUBLIC_API_URL ?? defaultApiUrl).replace(/\/$/, "");

export async function compareCode(code1: string, code2: string): Promise<CompareResult> {
  let response: Response;
  try {
    response = await fetch(`${API_URL}/compare`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ code1, code2 }),
    });
  } catch {
    throw new Error(
      "The backend is unreachable. Make sure it is running and try again.",
    );
  }

  let payload: CompareResult | ErrorResponse;
  try {
    payload = (await response.json()) as CompareResult | ErrorResponse;
  } catch {
    throw new Error("The backend returned an unreadable response.");
  }

  if (!response.ok) {
    const message = "error" in payload ? payload.error : undefined;
    throw new Error(message || `Comparison failed with status ${response.status}.`);
  }

  return payload as CompareResult;
}
