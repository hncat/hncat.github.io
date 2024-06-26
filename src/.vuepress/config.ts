import { defineUserConfig } from "vuepress";

import theme from "./theme.js";

export default defineUserConfig({
  base: "/",

  lang: "zh-CN",
  title: "far",
  description: "生活的理想就是理想的生活。",

  theme,

  // 和 PWA 一起启用
  // shouldPrefetch: false,
});
