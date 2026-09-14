import { render, screen, waitFor } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { beforeEach, describe, expect, it, vi } from "vitest";

import Home from "./page";
import { compareCode } from "@/lib/api";

vi.mock("@/lib/api", () => ({
  compareCode: vi.fn(),
}));

const mockedCompareCode = vi.mocked(compareCode);

describe("comparison page", () => {
  beforeEach(() => {
    mockedCompareCode.mockReset();
    mockedCompareCode.mockResolvedValue({
      similarity: 100,
      matchedLines1: [[1, 1]],
      matchedLines2: [[1, 1]],
      totalFingerprints1: 1,
      totalFingerprints2: 1,
      matchedFingerprints: 1,
    });
  });

  it("enables comparison after both inputs are filled and calls the API", async () => {
    const user = userEvent.setup();
    render(<Home />);

    const button = screen.getByRole("button", { name: "Compare code" });
    expect(button).toBeDisabled();

    await user.type(screen.getByLabelText("File A code"), "int main return 1");
    expect(button).toBeDisabled();
    await user.type(screen.getByLabelText("File B code"), "int run return 2");
    expect(button).toBeEnabled();

    await user.click(button);

    await waitFor(() => {
      expect(mockedCompareCode).toHaveBeenCalledWith(
        "int main return 1",
        "int run return 2",
      );
    });
    expect(await screen.findByText("100%")).toBeInTheDocument();
  });

  it("shows a plain error when the backend is unreachable", async () => {
    mockedCompareCode.mockRejectedValue(
      new Error("The backend is unreachable. Make sure it is running and try again."),
    );
    const user = userEvent.setup();
    render(<Home />);

    await user.type(screen.getByLabelText("File A code"), "int first return 1");
    await user.type(screen.getByLabelText("File B code"), "int second return 2");
    await user.click(screen.getByRole("button", { name: "Compare code" }));

    expect(await screen.findByRole("alert")).toHaveTextContent(
      "The backend is unreachable. Make sure it is running and try again.",
    );
  });
});
