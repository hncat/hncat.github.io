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
      children: [{
        text: "链接、装载和库",
        icon: "book",
        prefix: "linker/",
        link: "linker/",
        children: "structure"
      }],
    },
    "intro",
  ],
});
