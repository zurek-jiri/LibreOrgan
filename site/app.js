"use strict";
const config = window.libreOrganSite || {};
const views = {
  main: { file: "main.jpg", alt: "The main window with the Stralsund, St. Jakobi organ: four divisions with their stop buttons, stop folders, extensions and transmissions, the voicing bars under the stops, couplers and tremulants in the headers, the swell shutters at different positions, a combination engaged on piston 7, the crescendo and the polyphony display above.", caption: "The console: one column per division, the stops as buttons, couplers and tremulants in the headers, pistons and the crescendo above. Shown: Stralsund, St. Jakobi, Wegscheider organ (Sonus Paradisi), four divisions - combination 7 drawn, the orange bars under the stops are their voicing, the swell boxes of the divisions at different positions, the Positive's tremulant on; with stop folders (Cornet [III], Vox Humana Cornet [V]), extensions (a 64' pedal from the 32', Montre 32', Septime 1 1/7') and transmissions (Flöte 2', Regal 16') the organ never had, all built in the Stop List from its own ranks." },
  stoplist: { file: "stoplist.png", alt: "The Stop List: a table of stops per division with their sample folders per perspective and the transmissions.", caption: "The Stop List: the organ is built here from the loaded folder trees - stops, perspectives, divisions, transmissions." },
  divisions: { file: "divisions.png", alt: "The Divisions dialog: MIDI, compass, swell, tremulant and wind of a division, and its audio tabs with the device, buffer size and output channels per perspective.", caption: "The Divisions dialog: MIDI and compass, swell, tremulant, wind - and one audio route per perspective, each on any device and channels." },
  editor: { file: "editor.png", alt: "The Configuration Editor: the engine, global sound and console options, each with a question mark that opens the manual at its chapter.", caption: "The Configuration Editor: every option explained by the manual chapter behind its question mark." }
};
for (const button of document.querySelectorAll("[data-view]")) {
  button.addEventListener("click", () => {
    const videoFigure = document.getElementById("video-figure"), screenshotFigure = document.getElementById("screenshot-figure"), video = document.getElementById("demo-video");
    if (button.dataset.view === "video") {
      for (const peer of document.querySelectorAll("[data-view]")) peer.setAttribute("aria-pressed", String(peer === button));
      screenshotFigure.hidden = true;
      videoFigure.hidden = false;
      return;
    }
    const view = views[button.dataset.view];
    if (!view) return;
    for (const peer of document.querySelectorAll("[data-view]")) peer.setAttribute("aria-pressed", String(peer === button));
    if (video) video.pause();
    videoFigure.hidden = true;
    screenshotFigure.hidden = false;
    const img = document.getElementById("screenshot");
    img.src = "assets/" + view.file;
    img.alt = view.alt;
    for (const id of ["screenshot-link", "full-size"]) document.getElementById(id).href = img.src;
    document.getElementById("screenshot-caption").textContent = view.caption;
  });
}
for (const el of document.querySelectorAll("[data-version]")) el.textContent = config.version || "";
const validRepository = typeof config.repository === "string" && /^[A-Za-z0-9][A-Za-z0-9-]*\/[A-Za-z0-9_.-]+$/.test(config.repository);
const repo = validRepository ? "https://github.com/" + config.repository : "https://github.com/zurek-jiri/LibreOrgan";
for (const el of document.querySelectorAll("[data-repo-path]")) el.href = repo + "/blob/main/" + el.dataset.repoPath;
for (const el of document.querySelectorAll("[data-repo-tree]")) el.href = repo + "/tree/main/" + el.dataset.repoTree;
for (const el of document.querySelectorAll("[data-repo-suffix]")) el.href = repo + el.dataset.repoSuffix;
