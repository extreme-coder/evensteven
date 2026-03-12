import {
  AreaChart,
  Area,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ReferenceLine,
  ResponsiveContainer,
  Legend,
} from 'recharts'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import type { SongAnalysis } from '@/lib/store'

interface Props {
  song: SongAnalysis
  viewMode: 'beginner' | 'advanced'
}

function formatTime(seconds: number) {
  const m = Math.floor(seconds / 60)
  const s = Math.floor(seconds % 60)
  return `${m}:${s.toString().padStart(2, '0')}`
}

export default function BalanceChart({ song, viewMode }: Props) {
  const data = song.balance.timestamps.map((t, i) => ({
    time: t,
    vocal: song.balance.vocal_energy_db[i] ?? null,
    accompaniment: song.balance.accompaniment_energy_db[i] ?? null,
    balance: viewMode === 'advanced' ? (song.balance.balance_db[i] ?? null) : null,
    masked: song.balance.masking_risk[i] ? -100 : null,
  }))

  const maxPoints = 500
  const step = Math.max(1, Math.floor(data.length / maxPoints))
  const sampled = data.filter((_, i) => i % step === 0)

  return (
    <Card>
      <CardHeader className="pb-2">
        <CardTitle className="text-base">
          Balance: Vocal vs Accompaniment
        </CardTitle>
      </CardHeader>
      <CardContent>
        <ResponsiveContainer width="100%" height={280}>
          <AreaChart data={sampled} margin={{ top: 5, right: 20, bottom: 5, left: 0 }}>
            <CartesianGrid strokeDasharray="3 3" stroke="var(--border)" />
            <XAxis
              dataKey="time"
              tickFormatter={formatTime}
              type="number"
              domain={['dataMin', 'dataMax']}
              tick={{ fontSize: 11, fill: 'var(--muted-foreground)' }}
            />
            <YAxis
              tick={{ fontSize: 11, fill: 'var(--muted-foreground)' }}
              domain={['auto', 'auto']}
              label={{ value: 'dB', angle: -90, position: 'insideLeft', style: { fontSize: 11, fill: 'var(--muted-foreground)' } }}
            />
            <Tooltip
              labelFormatter={(v) => formatTime(Number(v))}
              formatter={(v, name) => [Number(v).toFixed(1) + ' dB', name]}
              contentStyle={{ backgroundColor: 'var(--popover)', border: '1px solid var(--border)', color: 'var(--popover-foreground)' }}
            />
            <Legend />
            <ReferenceLine y={-3} stroke="var(--destructive)" strokeDasharray="5 5" label="" />
            <Area
              type="monotone"
              dataKey="vocal"
              stroke="var(--chart-1)"
              fill="var(--chart-1)"
              fillOpacity={0.15}
              strokeWidth={1.5}
              dot={false}
              name="Vocal"
            />
            <Area
              type="monotone"
              dataKey="accompaniment"
              stroke="var(--chart-5)"
              fill="var(--chart-5)"
              fillOpacity={0.1}
              strokeWidth={1.5}
              dot={false}
              name="Accompaniment"
            />
          </AreaChart>
        </ResponsiveContainer>
      </CardContent>
    </Card>
  )
}
