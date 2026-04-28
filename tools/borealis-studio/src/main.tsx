import React from "react";
import ReactDOM from "react-dom/client";
import { BorealisStudioApp } from "./App";
import "./styles/index.css";

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    <BorealisStudioApp />
  </React.StrictMode>,
);
