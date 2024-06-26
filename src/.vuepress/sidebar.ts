import { sidebar } from "vuepress-theme-hope";

export default sidebar({
  "/": [
    "",
    {
      text: "c/c++",
      icon: "code",
      prefix: "code/",
      link: "code/",
      children: "structure",
    },
    {
      text: "计算机基础",
      icon: "book",
      prefix: "cs/",
      link: "cs/",
      children: "structure",
    },
    "intro",
  ],
});
