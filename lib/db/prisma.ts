import { PrismaClient } from "@prisma/client";
import fs from "fs";
import path from "path";

function getDatabaseUrl(): string | undefined {
  if (process.env.VERCEL && (!process.env.DATABASE_URL || process.env.DATABASE_URL.includes("dev.db"))) {
    const tmpPath = "/tmp/dev.db";
    const srcPath = path.join(process.cwd(), "prisma", "dev.db");
    try {
      if (!fs.existsSync(tmpPath) && fs.existsSync(srcPath)) {
        fs.copyFileSync(srcPath, tmpPath);
      }
      return `file:${tmpPath}`;
    } catch (e) {
      console.warn("Notice: SQLite /tmp fallback check:", e);
    }
  }
  return undefined;
}

const overrideUrl = getDatabaseUrl();

const globalForPrisma = globalThis as unknown as {
  prisma: PrismaClient | undefined;
};

export const prisma =
  globalForPrisma.prisma ??
  new PrismaClient({
    datasources: overrideUrl ? { db: { url: overrideUrl } } : undefined,
    log: process.env.NODE_ENV === "development" ? ["warn", "error"] : ["error"],
  });

if (process.env.NODE_ENV !== "production") globalForPrisma.prisma = prisma;
